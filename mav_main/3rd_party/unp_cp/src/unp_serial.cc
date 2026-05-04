#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/poll.h>

#include "unp.h"
#include "unp_threads.h"
#include "unp_time.h"
#include "unp_private.h"
#include "unp_log.h"
#include "unp_misc/io.h"

#include "unp_serial.h"

namespace UNP {

SerialPort::~SerialPort() {
	Close();
}

static void SerialTermiosInit(int fd, termios &t) {
	if(tcgetattr(fd, &t) != 0) {
		l.AddSysError(errno, "tcgetattr");
		throw errno;
	}
	
	t.c_cflag |= ( CLOCAL | CREAD );
	t.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY );
	t.c_oflag &= ~( OPOST | OLCUC | ONLCR | OCRNL );
	t.c_lflag &= ~( ECHO | ECHOE | ECHOK | NOFLSH | ECHONL | ICANON | ISIG | IEXTEN );
	t.c_cflag &= ~CSIZE;
	t.c_cflag |= ( CS8 );
	t.c_cflag &= ~( CSTOPB | PARENB
#ifdef CMSPAR
		| CMSPAR
#endif
		| PARODD );
	t.c_cc[ VMIN ]  = 1;
	t.c_cc[ VTIME ] = 0;
	
	// INPCK - проверка на чётность принимаемых байт
	if(t.c_cflag & PARENB)
		t.c_iflag |= (INPCK);
	else
		t.c_iflag &= ~INPCK;
}

#define DATA_BITS_CASE(v) \
	case v: \
		t.c_cflag |= CS ## v; \
		break;

#define SPEED_CASE(v) \
	case v: \
		return B##v;

static speed_t GetSpeed(int speed) {
#ifdef __QNXNTO__
	static_assert(B9600 == 9600, "");
	return speed;
#else
	switch(speed) {
		SPEED_CASE(300)
		SPEED_CASE(600)
		SPEED_CASE(1200)
		SPEED_CASE(1800)
		SPEED_CASE(2400)
		SPEED_CASE(4800)
		SPEED_CASE(9600)
		SPEED_CASE(19200)
		SPEED_CASE(38400)
		SPEED_CASE(57600)
		SPEED_CASE(115200)
		SPEED_CASE(230400)
		SPEED_CASE(460800)
		SPEED_CASE(500000)
		SPEED_CASE(576000)
		SPEED_CASE(921600)
		SPEED_CASE(1000000)
		SPEED_CASE(1152000)
		SPEED_CASE(1500000)
		SPEED_CASE(2000000)
		SPEED_CASE(2500000)
		SPEED_CASE(3000000)
		SPEED_CASE(3500000)
		SPEED_CASE(4000000)
		
		default:
			l.AddError("Wrong speed %d", speed);
			throw EINVAL;
	}
#endif
}

static void SerialSet(int fd, termios &t, const SerialSets &sets) {
	speed_t iSpeed, oSpeed;
	
	iSpeed = GetSpeed(sets.iSpeed);
	oSpeed = GetSpeed(sets.oSpeed);
	
	if(cfsetispeed(&t, iSpeed) != 0) {
		l.AddSysError(errno, "cfsetispeed");
		throw EINVAL;
	}
	
	if(cfsetospeed(&t, oSpeed) != 0) {
		l.AddSysError(errno, "cfsetospeed");
		throw EINVAL;
	}
	
	t.c_cflag &= ~(CSIZE);
	switch(sets.dataBits) {
		DATA_BITS_CASE(5)
		DATA_BITS_CASE(6)
		DATA_BITS_CASE(7)
		DATA_BITS_CASE(8)
		
		default:
			l.AddError("Wrong data bits %d", sets.dataBits);
			throw EINVAL;
	}
	
	switch(sets.parity) {
		case SERIAL_PARITY_NONE:
			t.c_cflag &= ~(PARENB|PARODD);
			break;
		case SERIAL_PARITY_EVEN:
			t.c_iflag |= INPCK;
			t.c_cflag |= PARENB;
			t.c_cflag &= ~PARODD;
			break;
		case SERIAL_PARITY_ODD:
			t.c_iflag |= INPCK;
			t.c_cflag |= (PARENB | PARODD);
			break;
		default:
			l.AddError("Wrong parity");
			throw EINVAL;
	}
	
	switch(sets.stopBits) {
		case 1:
			t.c_cflag &= ~(CSTOPB);
			break;
		case 2:
			t.c_cflag |= CSTOPB;
			break;
		default:
			l.AddError("Wrong stop bits count %d", sets.stopBits);
			throw EINVAL;
	}
	
//	t.c_cflag &= ~CRTSCTS; // отключим аппаратное управление потоком
	
	if(tcsetattr(fd, TCSADRAIN, &t) != 0) {
		l.AddSysError(errno, "tcsetattr");
		throw errno;
	}
}

void SerialPort::Open(const char *path, const SerialSets &sets, bool nonBlock) {
	Close();
	
	if(!path)
		path = "";
	
	int flags = O_RDWR | O_NOCTTY;
	
	if(nonBlock)
		flags |= O_NONBLOCK;
	
	fd = open(path, flags, 0);
	if(fd < 0) {
		l.AddError("Can't open '%s': %s", path, strerror(errno));
		throw errno;
	}
	
	Discard();
	
	static_assert(sizeof(termCur) >= sizeof(termios), "");
	
	termios *term = reinterpret_cast<termios*>(termCur);
//	termios term;
	
	SerialTermiosInit(fd, *term);
	SerialSet(fd, *term, sets);
	
	speedMax = std::max(sets.iSpeed, sets.oSpeed);
}

void SerialPort::Close() {
	if(fd >= 0) {
		UNP::ThreadCancelDisabler d;
		
		Drain();
		close(fd);
		fd = -1;
		
		if(wasTransmited) {
			needPause = true;
			tLastTransmit_us = UNP::GetTime(UNP::TM_US);
		}
	}
}

void SerialPort::ChangeSerialSets(const SerialSets &sets) {
	termios *term = reinterpret_cast<termios*>(termCur);
	SerialSet(fd, *term, sets);
	speedMax = std::max(sets.iSpeed, sets.oSpeed);
}

void SerialPort::Discard() {
	if(tcflush(fd, TCIOFLUSH) != 0)
		l.AddSysError(errno, "tcflush");
}

void SerialPort::Drain() {
	if(tcdrain(fd) != 0)
		l.AddSysError(errno, "tcdrain");
}

void SerialPort::SleepIfNeed() {
	if(needPause && speedMax > 0) {
		int64_t t, tSleep, tPeriod; // us
		
		tPeriod = 16 * 8 * 1000000LL / speedMax; // Интервал 16 байт
		t = UNP::GetTime(UNP::TM_US);
		tSleep = t > tLastTransmit_us ? (tPeriod - (t - tLastTransmit_us)) : tPeriod;
		
		if(tSleep > 0.0) {
			usleep(tSleep);
		}
		
		needPause = false;
	}
}

size_t SerialPort::Read(void *buf, size_t size, int timeout_ms, bool logTo) {
	if(buf && size > 0) {
		pollfd pollsd;
		int r;
		
		SleepIfNeed();
		wasTransmited = true;
		
		pollsd.fd = fd;
		pollsd.events = POLLIN | POLLHUP;
		pollsd.revents = 0;
		
		r = poll(&pollsd, 1, timeout_ms);
		if(r == 0) { // timeout
			if(logTo)
				l.AddWarning("serial: poll: timeout");
			throw ETIMEDOUT;
		}
		else if(r < 0) {
			l.AddSysError(errno, "serial: poll");
			throw errno;
		}
		
//		printf("poll: r=%d revents=0x%X\n", r, pollsd.revents);
//		fflush(stdout);
		
		if(pollsd.revents & POLLNVAL) {
			l.AddError("%s: %s", "serial: poll", "POLLNVAL");
			throw EBADF;
		}
		
		if(pollsd.revents & POLLERR) {
			l.AddError("%s: %s", "serial: poll", "POLLERR");
			throw EIO;
		}
		
		if(pollsd.revents & POLLHUP) {
			l.AddError("%s: %s", "serial: poll", "POLLHUP");
			throw EIO;
		}
		
		if(!(pollsd.revents & POLLIN)) {
			// Сюда попасть не должны
			l.AddError("%s: %s", "serial: poll", "not POLLIN");
			throw ENODATA;
		}
		
		while( (r = read(fd, buf, size)) < 0 && errno == EINTR ) {
//			printf("read: loop: %d %d\n", r, errno);
//			fflush(stdout);
		}
		
//		printf("read: brk: %d %d\n", r, errno);
//		fflush(stdout);
		
		if(r < 0) {
			l.AddSysError(errno, "serial: read");
			throw errno;
		}
		else if(r == 0) { // eof
			l.AddWarning("serial: read: no data");
			throw ENODATA;
		}
		
		if(verbose)
			LogDump("Serial", "<--", buf, r);
		
		return r;
	}
	else {
		return 0;
	}
}

void SerialPort::ReadN(void *vptr, size_t n, int timeoutFirst_ms, int timeoutNext_ms, bool logToFirst, bool logToNext) {
	unsigned nleft;
	size_t nread;
	uint8_t *ptr;
	bool first = true;
	int timeout_ms;
	bool logTo;
	
	ptr = static_cast<uint8_t*>(vptr);
	nleft = n;
	
	while(nleft > 0) {
		if(first) {
			timeout_ms = timeoutFirst_ms;
			logTo = logToFirst;
		}
		else {
			timeout_ms = timeoutNext_ms;
			logTo = logToNext;
		}
		
		nread = Read(ptr, nleft, timeout_ms, logTo);
		
		nleft -= nread;
		ptr += nread;
		first = false;
	}
}

static void WriteN(int fd, const void *buf_, size_t len) {
	int r = UNP::writen(fd, buf_, len);
	
	if(r < 0) {
		l.AddSysError(errno, "serial: writen");
		throw errno;
	}
	else if(r != (int)len) {
		// Сюда попасть не должны
		l.AddError("Write serial %d bytes instead %d", r, (int)len);
		throw EBADE;
	}
}

void SerialPort::WriteN(const void *buf_, size_t len) {
	if(len > 0) {
		SleepIfNeed();
		wasTransmited = true;
		
		if(verbose)
			LogDump("Serial", "-->", buf_, len);
		
		UNP::WriteN(fd, buf_, len);
	}
}

}
