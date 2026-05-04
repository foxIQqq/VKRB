/*
 * unp_fs.cc
 *
 *  Created on: 21.08.2015
 *      Author: kudr
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "unp_misc/io.h"
#include "unp_private.h"

#include "unp_fs.h"

#define PATH_SEP '/'

namespace UNP {

bool MkDir(const char *path) {
	if(!path || !path[0]) {
		l.AddError("MkDir: Path is empty");
		return false;
	}
	
	if(mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0)
		return true;
	else {
		l.AddSysError(errno, "mkdir", path);
		return false;
	}
}

bool IsDir(const char *path) {
	if(!path || !path[0]) {
		l.AddError("IsDir: Path is empty");
		return false;
	}
	
	struct stat buf;
	if(stat(path, &buf) == 0)
		return S_ISDIR(buf.st_mode);
	else {
		if(errno != ENOENT)
			l.AddSysError(errno, "stat", path);
		return false;
	}
}

bool IsReg(const char *path) {
	if(!path || !path[0]) {
		l.AddError("IsReg: Path is empty");
		return false;
	}
	
	struct stat buf;
	if(stat(path, &buf) == 0)
		return S_ISREG(buf.st_mode);
	else {
		if(errno != ENOENT)
			l.AddSysError(errno, "stat", path);
		return false;
	}
}

bool FileNotExist(const char *path) {
	if(!path || !path[0]) {
		l.AddError("FileNotExist: Path is empty");
		return false;
	}
	
	struct stat buf;
	if(stat(path, &buf) == 0)
		return false;
	else {
		if(errno == ENOENT)
			return true;
		else {
			l.AddSysError(errno, "stat", path);
			return false;
		}
	}
}

bool MkDirA(const char *path) {
	if(!path || !path[0]) {
		l.AddError("MkDirA: Path is empty");
		return false;
	}
	
	if(IsDir(path))
		return true;

	const char *sep;
	std::string sub;
	bool ok = true;
	
	for(sep = path; ok;) {
		sep = strchr(sep + 1, PATH_SEP);
		if(sep) {
			sub = "";
			sub.append(path, sep - path);
			
			ok = IsDir(sub.c_str()) ? true : MkDir(sub.c_str());
			sep = sep + 1;
		}
		else {
			ok = IsDir(path) ? true : MkDir(path);
			break;
		}
	}
	
	return ok;
}

static bool RemoveRecurs(const char *path) {
	int r;
	
	if(IsDir(path)) {
		DIR *dirp;
		struct dirent* direntp;
		std::string pathChild;
		const char *dname;
		
		dirp = opendir(path);
		if(dirp == NULL) {
			l.AddSysError(errno, "opendir", path);
			return false;
		}
		
		for(;;) {
			direntp = readdir(dirp);
			if(direntp == NULL)
				break;
			dname = direntp->d_name;
			if(strcmp(dname, ".") != 0 && strcmp(dname, "..") != 0) {
				pathChild = path;
				pathChild += "/";
				pathChild += dname;
				if(!RemoveRecurs(pathChild.c_str())) {
					closedir(dirp);
					return false;
				}
			}
		}
		
		closedir(dirp);
		
		r = rmdir(path);
		if(r != 0 && errno != ENOENT) {
			l.AddSysError(errno, "rmdir", path);
			return false;
		}
	}
	else {
		r = unlink(path);
		if(r != 0 && errno != ENOENT) {
			l.AddSysError(errno, "unlink", path);
			return false;
		}
	}
	
	return true;
}

bool RemoveA(const char *path) {
	if(!path || !path[0]) {
		l.AddError("RemoveA: Path is empty");
		return false;
	}
	
	return RemoveRecurs(path);
}

bool CopyFile(const char *src, const char *dst, const volatile bool *pwork) {
	if(!src || !src[0] || !dst || !dst[0]) {
		l.AddError("CopyFile: Path is empty");
		return false;
	}
	
	unlink(dst);
	
	bool ok = true;
	const int BUF_SIZE = 512 * 1024;
	uint8_t *buf = NULL;
	
	int fSrc = -1, fDst = -1;
	
	try {
		fSrc = open64(src, O_RDONLY, 0);
		if(fSrc < 0) {
			l.AddError("CopyFile: Can't open src file '%s': %s", src, strerror(errno));
			throw 0;
		}
		
		int mode = 0;
		
		{
			struct stat64 st;
			if(fstat64(fSrc, &st) == 0)
				mode = st.st_mode;
		}
		
		fDst = open64(dst, O_CREAT | O_WRONLY, mode);
		if(fDst < 0) {
			l.AddError("CopyFile: Can't open dst file '%s': %s", src, strerror(errno));
			throw 0;
		}
		
		buf = new uint8_t[BUF_SIZE];
		
		ssize_t nr;
		
		while((nr = readn(fSrc, buf, BUF_SIZE)) > 0) {
			if(pwork && !*pwork) {
				l.AddWarning("CopyFile: canceled");
				throw 0;
			}
			
			ssize_t nw = writen(fDst, buf, nr);
			if(nw != nr) {
				if(nw < 0)
					l.AddSysError(errno, "CopyFile: write");
				else
					l.AddError("CopyFile: write only %d/%d", int(nw), int(nr));
				throw 0;
			}
			
			if(pwork && !*pwork) {
				l.AddWarning("CopyFile: canceled");
				throw 0;
			}
		}
		
		if(nr < 0) {
			l.AddSysError(errno, "CopyFile: read");
			throw 0;
		}
	}
	catch(...) {
		ok = false;
	}
	
	if(fDst >= 0)
		close(fDst);
	
	if(fSrc >= 0)
		close(fSrc);
	
	delete []buf;
	
	return ok;
}

}
