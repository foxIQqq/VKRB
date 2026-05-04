#include <unistd.h>
#include <fcntl.h>

#include "unp_misc/lock.h"

namespace UNP {

static int lock_reg(int fd, int cmd, int type) {
	struct flock lock{};
	
	lock.l_type = type;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	
	return fcntl(fd, cmd, &lock);
}

int read_lock(int fd) {
	return lock_reg(fd, F_SETLK, F_RDLCK);
}

int readw_lock(int fd) {
	return lock_reg(fd, F_SETLKW, F_RDLCK);
}

int write_lock(int fd) {
	return lock_reg(fd, F_SETLK, F_WRLCK);
}

int writew_lock(int fd) {
	return lock_reg(fd, F_SETLKW, F_WRLCK);
}

int un_lock(int fd) {
	return lock_reg(fd, F_SETLK, F_UNLCK);
}

}
