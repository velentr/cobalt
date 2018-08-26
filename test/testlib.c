/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>

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

int __mkdir_fail = 0;
extern int __real_mkdir(const char *path, mode_t mode);
int __wrap_mkdir(const char *path, mode_t mode)
{
	if (__mkdir_fail) {
		errno = __mkdir_fail;
		return -1;
	} else {
		return __real_mkdir(path, mode);
	}
}

int __open_fail = 0;
extern int __real_open(const char *pathname, int flags, mode_t mode);
int __wrap_open(const char *path, int flags, mode_t mode)
{
	if (__open_fail) {
		errno = __open_fail;
		return -1;
	} else {
		return __real_open(path, flags, mode);
	}
}

int __write_fail = 0;
size_t __write_partial = 0;
extern ssize_t __real_write(int fd, const void *buf, size_t count);
ssize_t __wrap_write(int fd, const void *buf, size_t count)
{
	if (__write_fail) {
		errno = __write_fail;
		return -1;
	} else if (__write_partial) {
		return __real_write(fd, buf, __write_partial);
	} else {
		return __real_write(fd, buf, count);
	}
}

int __rename_fail = 0;
extern int __real_rename(const char *old, const char *new);
int __wrap_rename(const char *old, const char *new)
{
	if (__rename_fail) {
		errno = __rename_fail;
		return -1;
	} else {
		return __real_rename(old, new);
	}
}

int __symlink_fail = 0;
extern int __real_symlink(const char *target, const char *linkpath);
int __wrap_symlink(const char *target, const char *linkpath)
{
	if (__symlink_fail) {
		errno = __symlink_fail;
		return -1;
	} else {
		return __real_symlink(target, linkpath);
	}
}

int __unlink_fail = 0;
extern int __real_unlink(const char *path);
int __wrap_unlink(const char *path)
{
	if (__unlink_fail) {
		errno = __unlink_fail;
		return -1;
	} else {
		return __real_unlink(path);
	}
}

