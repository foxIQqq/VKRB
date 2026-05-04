#include <malloc.h>

#include "unp_misc/malloc.h"

namespace UNP {

void* realloc(void *ptr, size_t size) {
	// Проверяю size из-за поведения realloc() при size == 0 
	if(size <= 0) {
		if(ptr)
			::free(ptr);
		return NULL;
	}
	
	void *p = ::realloc(ptr, size);
	if(p == NULL && ptr)
		::free(ptr);
	return p;
}

}
