/*
 * daemonize.cc
 *
 *  Created on: 15.10.2012
 *      Author: kudr
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#ifdef __QNX__
#include <sys/procmgr.h>
#else
#include <sys/resource.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#endif

#include "unp_misc/io.h"
#include "unp_misc/lock.h"

#include "unp_misc/daemonize.h"

namespace UNP {

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap) {
	int oldCancelState;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState);
	
	fflush(stdout); // в случае, когда stdout и stderr - одно и то же устройство
	
	vfprintf(stderr, fmt, ap);
	if(errnoflag) {
		fprintf(stderr, ": %d - '%s'", error, strerror(error));
	}
	fprintf(stderr, "\n");
	
	fflush(NULL); // сбрасывает все входные потоки
	
	pthread_setcancelstate(oldCancelState, NULL);
}

static void err_quit(const char *fmt, ...) {
	va_list ap;
	
	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

static void err_ret(const char *fmt, ...) {
	va_list ap;
	
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

static void err_msg(const char *fmt, ...) {
	va_list ap;
	
	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}

void daemonize(uint32_t flags) {
#ifdef __QNX__
	int r;
	unsigned f = 0;
	
	if(flags & UNP_DAEMONIZE_NOCHDIR)
		f |= PROCMGR_DAEMON_NOCHDIR;
	
	r = procmgr_daemon(EXIT_SUCCESS, f);
	if(r == -1)
		err_quit("Call '%s' error", "procmgr_daemon");
#else
#if 0
    int r;

    r = daemon(flags & UNP_DAEMONIZE_NOCHDIR ? 1 : 0, 0);
    if(r == -1)
        err_quit("Call '%s' error", "daemon");
#else
	struct rlimit rl;
	pid_t pid;
	struct sigaction sa;
	int i, fd0, fd1, fd2;
	
	bzero(&sa, sizeof(sa));
	
	// Сбросить маску режима создания файла.
	umask(0);
	
	// Получить максимально возможный номер дескриптора файла.
	if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
		err_quit("Can't get maximum descriptor number");
	
	// Стать лидером новой сессии, чтобы утратить управляющий терминал.
	if((pid = fork()) < 0)
		err_quit("Call '%s' error", "fork");
	else if(pid != 0) // родительский процесс
		exit(0);
	//
	setsid();
	
	// Обеспечить невозможность обретения управляющего терминала в будущем.
	/*sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP, &sa, NULL) < 0)
		err_quit("Can't ignore signal SIGHUP");*/
	//
	if((pid = fork()) < 0)
		err_quit("Call '%s' error", "fork");
	else if(pid != 0) // родительский процесс
		exit(0);
	
	if(!(flags & UNP_DAEMONIZE_NOCHDIR)) {
		// Назначить корневой каталог текущим рабочим каталогом, чтобы впоследствии можно было отмонтировать файловую систему.
		if(chdir("/") < 0)
			err_quit("Can't set work dir '/'");
	}
	
	// Закрыть все открытые файловые дескрипторы
	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for(i = 0; i < (int)rl.rlim_max; i++)
		close(i);
	
	// Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
	
	if(fd0 != 0 || fd1 != 1 || fd2 != 2) {
		syslog(LOG_ERR, "Wrong file descriptors %d %d %d", fd0, fd1, fd2);
		exit(1);
	}
#endif
#endif
}

int already_running_by_lock(const char *lockFile, int extraOpenFlags, bool writePid, pid_t *ppid, int *pfd) {
	int fd;
	
	if(lockFile == NULL)
		lockFile = "";
	
	if(ppid)
		*ppid = -1;
	if(pfd)
		*pfd = -1;
	
	fd = open(lockFile, O_RDWR|O_CREAT|extraOpenFlags, LOCKMODE);
	if(fd < 0) {
		fprintf(stderr, "Can't open '%s': '%s'\n", lockFile, strerror(errno));
		return -1;
	}
	
	if(write_lock(fd) < 0) {
		if(errno == EACCES || errno == EAGAIN) {
			if(ppid)
				*ppid = read_pid(fd);
			close(fd);
			return 1;
		}
		fprintf(stderr, "Can't lock on '%s': '%s'\n", lockFile, strerror(errno));
		close(fd);
		return -1;
	}
	
	write_pid(fd, writePid ? getpid() : -1);
	// fd не закрываем, чтобы оставить блокировку
	if(pfd)
		*pfd = fd;
	return 0;
}

int already_running_by_proc_exist(const char *pidFile, pid_t *ppid) {
	FILE *f;
	int r;
	
	if(pidFile == NULL)
		pidFile = "";
	
	if(ppid)
		*ppid = -1;
	
	f = fopen(pidFile, "rb");
	if(f) {
		// Существует
		pid_t pid = read_pid(fileno(f));
		
		if(ppid)
			*ppid = pid;
		
		r = kill(pid, 0);
		if(r == 0)
			return 1;
		else if(errno != ESRCH) {
			err_ret("Check process exist error");
			return -1;
		}
		
		// Процесса с указанным pid не существует
		err_msg("PID file exist, but process with pid=%ld not exist", pid);
	}
	else {
		if(errno != ENOENT) {
			// Ошибка
			err_ret("Error open '%s'", pidFile);
			return -1;
		}
		
		// Не существует
	}
	
	// Запишем pid в файл
	f = fopen(pidFile, "wb");
	if(f) {
		write_pid(fileno(f), getpid());
		fclose(f);
	}
	
	return 0;
}

pid_t read_pid(int fd) {
	char buf[32];
	int r = readn(fd, buf, sizeof(buf) - 1);
	if(r >= 0) {
		buf[r] = 0;
		return strtol(buf, NULL, 10);
	}
	else
		return -1;
}

int write_pid(int fd, pid_t pid) {
	int r = ftruncate(fd, 0);
	
	if(r == 0)
		r = lseek(fd, 0, SEEK_SET);
	
	if(r == 0 && pid > 0) {
		char buf[32];
		size_t len;
		
		buf[0] = 0;
		snprintf(buf, sizeof(buf), "%ld\n", (long)pid);
		len = strlen(buf);
		if(writen(fd, buf, len) != (int)len)
			r = -1;
	}
	
	return r;
}

}
