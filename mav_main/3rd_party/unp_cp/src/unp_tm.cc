/*
 * unp_tm.cc
 *
 *  Created on: 26 апр. 2024 г.
 *      Author: kudr
 */

#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include <string>
#include <list>

#include "unp_fs.h"
#include "unp_private.h"

#include "unp_tm.h"

using namespace std;

#define EINTR_LOOP(ret, call) \
    do {                      \
        ret = call;           \
    } while (ret == -1 && errno == EINTR)

namespace UNP {

static inline unsigned GetFileId(const string &s) {
	const char *p = s.c_str();
	
	for(;*p && !isdigit(*p); ++p) {
	}
	
	return strtoul(p, NULL, 10);
}

static bool FilesCompare(const string &first, const string &second) {
	return GetFileId(first) < GetFileId(second);
}

static void GetNames(const char *dir, list<string> &names) {
	names.clear();
	
	DIR *dirp = opendir(dir);
	if(dirp) {
		for(;;) {
			dirent *direntp = readdir(dirp);
			if(direntp == NULL)
				break;
			
			if(strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
				continue;
			
			names.push_back(direntp->d_name);
		}
		
		closedir(dirp);
	}
	
	names.sort(FilesCompare);
}

Tm::Tm(const std::string &dir_, int digits_, const std::string &prefix_, const std::string &suffix_) : dir(dir_), prefix(prefix_), suffix(suffix_), digits(digits_), recId(0) {
	if(dir.empty())
		dir = ".";
	
	if(FileNotExist(dir.c_str())) {
		if(mkdir(dir.c_str(), 0777) != 0) {
			l.AddError("Can't create directory '%s': %s", dir.c_str(), strerror(errno));
		}
	}
	
	list<string> names;
	
	GetNames(dir.c_str(), names);
	
	if(!names.empty())
		recId = GetFileId(names.back());
}

string Tm::Next() {
	string path = dir;
	char buf[32];
	
	++recId;
	
	buf[0] = 0;
	
	if(!path.empty() && path[path.length() - 1] != '/')
		path += '/';
	
	path += prefix;
	snprintf(buf, sizeof(buf), "%0*u_", digits, recId);
	path += buf;
	
	struct tm tm = {0};
	time_t tCur = time(NULL); //-V795
	char fpgaTextBuf[100];
	localtime_r(&tCur, &tm);
	fpgaTextBuf[0] = 0;
	strftime(fpgaTextBuf, sizeof(fpgaTextBuf), "%Y%m%d_%H%M%S", &tm);
	
	path += fpgaTextBuf;
	path += suffix;
	
	return path;
}

static int64_t GetAvlSpace(const char *dir) {
	struct statvfs64 buf;
	int r;
	
	EINTR_LOOP(r, statvfs64(dir, &buf));
	if(r == 0) {
		return buf.f_bavail * buf.f_bsize;
	}
	else {
		l.AddSysError(errno, "statvfs64");
		return -1;
	}
}

void Tm::FreeSpace(uint64_t avlSize) {
	list<string> names;
	string path = dir;
	
	if(!path.empty() && path[path.length() - 1] != '/')
		path += '/';
	
	GetNames(dir.c_str(), names);
	
	while(!names.empty()) {
		int64_t avl = GetAvlSpace(dir.c_str());
		if(avl < 0 || avl >= (int64_t)avlSize)
			return;
		
		string s = path + *(names.begin());
		unlink(s.c_str());
		
		names.pop_front();
	}
}

}
