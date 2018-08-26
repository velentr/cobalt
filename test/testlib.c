/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdlib.h>

#include "testlib.h"

int __malloc_fail = 0;
extern void *__real_malloc(size_t size);
void *__wrap_malloc(size_t size)
{
	if (__malloc_fail) {
		errno = ENOMEM;
		return NULL;
	} else {
		return __real_malloc(size);
	}
}

int __realloc_fail = 0;
extern void *__real_realloc(void *ptr, size_t size);
void *__wrap_realloc(void *ptr, size_t size)
{
	if (__realloc_fail) {
		errno = ENOMEM;
		return NULL;
	} else {
		return __real_realloc(ptr, size);
	}
}

