/*
 * unp_log_sys.cc
 *
 *  Created on: 21 июн. 2022 г.
 *      Author: kudr
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef __QNXNTO__
#include <sys/slog.h>
#include <sys/slogcodes.h>
#else
#include <syslog.h>
#endif

#include <list>

#include "unp_misc/assert.h"
#include "unp_misc/lock.h"

#include "unp_log.h"

#define SEP "  "

#define LOG_COLOR_PREFIX_HIGHLIGHT  "\033[34;1m"
#define LOG_COLOR_PREFIX_SUCCESS    "\033[0;32m"
#define LOG_COLOR_PREFIX_WARNING    "\033[33;1m"
#define LOG_COLOR_PREFIX_ERROR      "\033[31;1m"

#define LOG_COLOR_SUFFIX            "\033[0m"

namespace UNP {

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

LoggerSys::LoggerSys(const LoggerSysSets &sets_) : sets(sets_) {
	Init();
}

LoggerSys::LoggerSys(const char *appName, bool toStd, bool toJournal, bool quiet, bool enColorConsole) {
	sets.appName = appName ? appName : "";
	sets.toStd = toStd;
	sets.toJournal = toJournal;
	sets.quiet = quiet;
	sets.enColorConsole = enColorConsole;
	
	Init();
}

// ptm - не NULL
static void GetLocalTime(struct tm *ptm) {
	time_t t = time(0); //-V795
	if(!localtime_r(&t, ptm))
		bzero(ptm, sizeof(*ptm));
}

// buf - не NULL, bufSize > 0
static void Time2Buf(char *buf, size_t bufSize, struct tm *ptm = NULL, bool forFileName = false) {
	buf[0] = 0;
	
	struct tm tm;
	if(!ptm)
		ptm = &tm;
	GetLocalTime(ptm);
	
	strftime(buf, bufSize, forFileName ? "%Y%m%d-%H%M%S" : "%Y-%m-%d %H:%M:%S", ptm);
}

static void Sync(FILE *f) {
	fflush(f);
	fsync(fileno(f));
}

void LoggerSys::Init() {
	logF = NULL;
	recordsCount = 0;
	tStart_ns = 0;
	
#ifndef __QNXNTO__
	if(sets.toJournal)
		openlog(sets.appName.c_str(), LOG_CONS, LOG_DAEMON);
#endif
	
	if(!sets.toFile.fileName.empty()) {
		logF = fopen(sets.toFile.fileName.c_str(), sets.toFile.truncBefore ? "wb" : "ab");
		if(logF) {
			char buf[32];
			
			struct stat st;
			if(fstat(fileno(logF), &st) == 0 && st.st_size > 0)
				fprintf(logF, "\n");
			
			Time2Buf(buf, sizeof(buf));
			fprintf(logF, "%s" SEP "UNP::LoggerSys: Log file '%s' opened\n", buf, sets.toFile.fileName.c_str());
			Sync(logF);
		}
	}
	
	sets.enColorConsole = sets.enColorConsole && isatty(fileno(stderr));
	
	if(sets.toStd && sets.stdAddTime) {
		timespec tspec;
		
		if(clock_gettime(CLOCK_MONOTONIC, &tspec) == 0) {
			tStart_ns = (int64_t)(tspec.tv_sec) * 1000000000 + tspec.tv_nsec;
		}
	}
	
	if(sets.toStd)
		lockFd = open("/var/lock/unp_std.lock", O_CREAT | O_RDWR, (S_IRUSR | S_IRGRP | S_IROTH) | (S_IWUSR | S_IWGRP | S_IWOTH));
	else
		lockFd = -1;
}

static std::string GetBackupFileName(const std::string &fileName, const char *bufTimeFile) {
	std::string s = fileName;
	
	s += "-";
	s += bufTimeFile;
	
	return s;
}

static std::string GetBackupList(std::list<std::string> &backupList, const std::string &fileName) {
	DIR *dirp;
	struct dirent *direntp;
	std::string dir, filePrefix;
	const char *dirSep;
	unsigned len;
	
	backupList.clear();
	
	dirSep = strrchr(fileName.c_str(), '/');
	
	if(dirSep)
		dir.append(fileName.c_str(), dirSep - fileName.c_str());
	else
		dir = ".";
	
	dirp = opendir(dir.c_str());
	if(dirp) {
		if(dirSep)
			filePrefix = dirSep + 1;
		else
			filePrefix = fileName;
		filePrefix += "-";
		
		for(;;) {
			direntp = readdir(dirp);
			if(!direntp)
				break;
			
			len = strlen(direntp->d_name);
			if(len >= filePrefix.length() && memcmp(filePrefix.c_str(), direntp->d_name, filePrefix.length()) == 0) {
				backupList.push_back(direntp->d_name);
			}
		}
		
		closedir(dirp);
	}
	
	return dir;
}

void LoggerSys::BackupLog() {
	if(logF) {
		char bufTimeText[32], bufTimeFile[32];
		struct tm tm;
		int r;
		std::string backupFileName, backupDir;
		std::list<std::string> backupList;
		
		GetLocalTime(&tm);
		
		Time2Buf(bufTimeText, sizeof(bufTimeText), &tm);
		Time2Buf(bufTimeFile, sizeof(bufTimeFile), &tm, true);
		
		backupFileName = GetBackupFileName(sets.toFile.fileName, bufTimeFile);
		
		// close
		fprintf(logF, "%s" SEP "UNP::LoggerSys: Log file '%s' closed and backup\n", bufTimeText, sets.toFile.fileName.c_str());
		
		Sync(logF);
		fclose(logF);
		logF = NULL;
		
		// backup
		
		/*cmd = "gzip < \"";
		cmd += sets.toFile.fileName;
		cmd += "\" > \"";
		cmd += backupFileName;
		cmd += "\"";
		system(cmd.c_str());*/
		
		backupDir = GetBackupList(backupList, sets.toFile.fileName);
		backupDir += "/";
		backupList.sort();
		
		{
			std::list<std::string>::const_iterator it;
			int i, c = backupList.size() - sets.toFile.backupCountMax + 1;
			bool ok = true;
			
			for(it = backupList.begin(), i = 0; it != backupList.end() && i < c; ++it, ++i) {
				r = unlink((backupDir + *it).c_str());
				ok = ok && r == 0;
			}
			
			if(ok)
				rename(sets.toFile.fileName.c_str(), backupFileName.c_str());
		}
		
		// open
		logF = fopen(sets.toFile.fileName.c_str(), "wb");
		if(logF) {
			char buf[32];
			Time2Buf(buf, sizeof(buf));
			fprintf(logF, "%s" SEP "UNP::LoggerSys: Log file '%s' truncated after backup to '%s'\n", buf, sets.toFile.fileName.c_str(), backupFileName.c_str());
			Sync(logF);
		}
	}
}

LoggerSys::~LoggerSys() {
	if(logF) {
		char buf[32];
		Time2Buf(buf, sizeof(buf));
		fprintf(logF, "%s" SEP "UNP::LoggerSys: Log file '%s' closed\n", buf, sets.toFile.fileName.c_str());
		Sync(logF);
		
		fclose(logF);
	}
	
#ifndef __QNXNTO__
	if(sets.toJournal)
		closelog();
#endif
	
	if(lockFd >= 0) {
		close(lockFd);
	}
}

void LoggerSys::AddV(LogMessageType type, const char *format, va_list ap) {
#ifndef DEBUG_VER
	if(type == LMT_DEBUG)
		return;
#endif
	
	if(!sets.quiet || type == LMT_ERROR) {
		va_list apCopy;
		bool useApCopyStd = true, useApCopyJournal = true;
		int oldCancelState;
		
		if(logF) {
		}
		else if(sets.toJournal)
			useApCopyJournal = false;
		else
			useApCopyStd = false;
		
		unp_assert(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState) == 0);
		unp_assert(pthread_mutex_lock(&mutex) == 0);
		
		if(sets.toStd) {
			FILE *f = stderr;
			const char *color = NULL;
			bool useCopy = useApCopyStd;
			
			if(useCopy)
				va_copy(apCopy, ap);
			
			if(sets.enColorConsole) {
				switch(type) {
					case LMT_HIGHLIGHT:
						color = LOG_COLOR_PREFIX_HIGHLIGHT;
						break;
					case LMT_SUCCESS:
						color = LOG_COLOR_PREFIX_SUCCESS;
						break;
					case LMT_WARNING:
						color = LOG_COLOR_PREFIX_WARNING;
						break;
					case LMT_ERROR:
						color = LOG_COLOR_PREFIX_ERROR;
						break;
					default:
						break;
				}
			}
			
			if(lockFd >= 0)
				writew_lock(lockFd);
			
			if(color)
				fprintf(f, "%s", color);
			
			if(!sets.appName.empty())
				fprintf(f, "%s: ", sets.appName.c_str());
			
			if(sets.stdAddTime) {
				timespec tspec;
				
				if(clock_gettime(CLOCK_MONOTONIC, &tspec) == 0) {
					constexpr int64_t NS = 1, US = 1000 * NS, MS = 1000 * US, S = 1000 * MS, MIN = 60 * S, HOUR = 60 * MIN;
					
					int64_t t_ns = (int64_t)(tspec.tv_sec) * 1000000000 + tspec.tv_nsec, t = t_ns - tStart_ns;
					int h, m, s, ms;
					
					h = t / HOUR;
					t %= HOUR;
					
					m = t / MIN;
					t %= MIN;
					
					s = t / S;
					t %= S;
					
					ms = t / MS;
					
					fprintf(f, "[%02d:%02d:%02d.%03d] ", h, m, s, ms);
				}
			}
			
			vfprintf(f, format, useCopy ? apCopy : ap);
			
			if(color)
				fprintf(f, "%s", LOG_COLOR_SUFFIX);
			fprintf(f, "\n");
			fflush(f);
			
			if(lockFd >= 0)
				un_lock(lockFd);
			
			if(useCopy)
				va_end(apCopy);
		}
		
		if(sets.toJournal) {
			bool useCopy = useApCopyJournal;
			
			if(useCopy)
				va_copy(apCopy, ap);
			
#ifdef __QNXNTO__
			int sev;
			
			switch(type) {
				case LMT_SUCCESS:
				case LMT_HIGHLIGHT:
				case LMT_NOTE:
					sev = _SLOG_INFO;
					break;
				case LMT_DEBUG:
					sev = _SLOG_DEBUG1;
					break;
				case LMT_WARNING:
					sev = _SLOG_WARNING;
					break;
				case LMT_ERROR:
					sev = _SLOG_ERROR;
					break;
				default:
					sev = _SLOG_INFO;
					break;
			}
			
			static char buf[1024]; // защищён mutex
			
			if(sets.appName.empty()) {
				vslogf(_SLOG_SETCODE(_SLOGC_TEST, 0), sev, format, useCopy ? apCopy : ap);
			}
			else {
				size_t len;
				
				buf[0] = 0;
				snprintf(buf, sizeof(buf), "%s: ", sets.appName.c_str());
				len = strlen(buf);
//				if(sizeof(buf) > len) // len гарантированно < размера буфера
					vsnprintf(buf + len, sizeof(buf) - len, format, useCopy ? apCopy : ap);
				
				slogf(_SLOG_SETCODE(_SLOGC_TEST, 0), sev, "%s", buf);
			}
#else
			int pri;
			
			switch(type) {
				case LMT_SUCCESS:
				case LMT_HIGHLIGHT:
				case LMT_NOTE:
					pri = LOG_INFO;
					break;
				case LMT_DEBUG:
					pri = LOG_DEBUG;
					break;
				case LMT_WARNING:
					pri = LOG_WARNING;
					break;
				case LMT_ERROR:
					pri = LOG_ERR;
					break;
				default:
					pri = LOG_INFO;
					break;
			}
			
			vsyslog(pri, format, useCopy ? apCopy : ap);
#endif
			
			if(useCopy)
				va_end(apCopy);
		}
		
		if(logF) {
			// Гарантированно не apCopy
			
			if(sets.toFile.recordsCountMax > 0) {
				if(recordsCount >= sets.toFile.recordsCountMax) {
					BackupLog();
					recordsCount = 0;
				}
			}
			
			if(!sets.toFile.noTime) {
				char buf[32];
				
				Time2Buf(buf, sizeof(buf));
				
				fprintf(logF, "%s" SEP, buf);
			}
			
			vfprintf(logF, format, ap);
			fprintf(logF, "\n");
			if(sets.toFile.sync)
				Sync(logF);
			else
				fflush(logF);
			
			if(sets.toFile.recordsCountMax > 0)
				++recordsCount;
		}
		
		unp_assert(pthread_mutex_unlock(&mutex) == 0);
		unp_assert(pthread_setcancelstate(oldCancelState, NULL) == 0);
	}
}

} // namespace
