#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "unp_misc/malloc.h"

#include "unp_misc/io.h"

namespace UNP {

ssize_t readn(int fd, void *vptr, size_t n) {
	unsigned nleft;
	int nread;
	char *ptr;
	
	ptr = static_cast<char*>(vptr);
	nleft = n;
	
	while(nleft > 0) {
		if((nread = read(fd, ptr, nleft)) < 0) {
			if(errno == EINTR)
				nread = 0; // и вызывает снова функцию read()
			else
				return -1; // ошибка
		}
		else if(nread == 0)
			break; // EOF
		
		nleft -= nread;
		ptr += nread;
	}
	
	return n - nleft; // возвращает значение >= 0
}

ssize_t writen(int fd, const void *vptr, size_t n) {
	unsigned nleft;
	int nwritten;
	const char *ptr;
	
	ptr = static_cast<const char*>(vptr);
	nleft = n;
	
	while(nleft > 0) {
		if((nwritten = write(fd, ptr, nleft)) <= 0) {
			if(errno == EINTR)
				nwritten = 0; // и снова вызывает функцию write()
			else
				return -1; // ошибка
		}
		
		nleft -= nwritten;
		ptr += nwritten;
	}
	
	return n;
}

ssize_t getline(char **line_, size_t *size_, FILE *stream) {
#ifdef __QNX__
#define line (*line_)
#define size (*size_)
	int v;
	int eolFounded = false;
	size_t sizeNew;
	size_t filled = 0; // без учёта 0 в конце
	
	do {
		sizeNew = filled + 1 + 1;
		if(sizeNew > size) {
			sizeNew += 128;
			
			line = static_cast<char*>(UNP::realloc(line, sizeNew));
			if(line) {
				size = sizeNew;
			}
			else {
				size = 0;
				break;
			}
		}
		
		if((v = fgetc(stream)) == EOF)
			break;
		
		eolFounded = v == '\n';
		
		line[filled] = v;
		++filled;
	} while(!eolFounded);
	
	line[filled] = 0;
	return filled;
#undef line
#undef size
#else
	return ::getline(line_, size_, stream);
#endif
}

}
