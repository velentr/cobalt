/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "list.h"
#include "util.h"

int __malloc_fail = 0;
int __malloc_fail_count = 0;
extern void *__real_malloc(size_t size);
void *__wrap_malloc(size_t size)
{
	if (__malloc_fail && __malloc_fail_count == 0) {
		errno = __malloc_fail;
		return NULL;
	} else if (__malloc_fail) {
		__malloc_fail_count--;
		return __real_malloc(size);
	} else {
		return __real_malloc(size);
	}
}

int __realloc_fail = 0;
extern void *__real_realloc(void *ptr, size_t size);
void *__wrap_realloc(void *ptr, size_t size)
{
	if (__realloc_fail) {
		errno = __realloc_fail;
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

int __openat_fail = 0;
extern int __real_openat(int dirfd, const char *path, int flags, mode_t mode);
int __wrap_openat(int dirfd, const char *path, int flags, mode_t mode)
{
	if (__openat_fail) {
		errno = __openat_fail;
		return -1;
	} else {
		return __real_openat(dirfd, path, flags, mode);
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

int __readdir_fail = 0;
extern struct dirent *__real_readdir(DIR *dirp);
struct dirent *__wrap_readdir(DIR *dirp)
{
	if (__readdir_fail) {
		errno = __readdir_fail;
		return NULL;
	} else {
		return __real_readdir(dirp);
	}
}

int __fstat_fail = 0;
extern int __real_fstat(int fd, struct stat *st);
int __wrap_fstat(int fd, struct stat *st)
{
	if (__fstat_fail) {
		errno = __fstat_fail;
		return -1;
	} else {
		return __real_fstat(fd, st);
	}
}

int __mmap_fail = 0;
extern void *__real_mmap(void *addr, size_t len, int prot, int flags, int fd,
		off_t offset);
void *__wrap_mmap(void *addr, size_t len, int prot, int flags, int fd,
		off_t offset)
{
	if (__mmap_fail) {
		errno = __mmap_fail;
		return MAP_FAILED;
	} else {
		return __real_mmap(addr, len, prot, flags, fd, offset);
	}
}

/* save all open fds on startup, and make sure no additional fds are open on
 * shutdown
 */
struct open_fd {
	struct list_elem le;
	int fd;
};

static LIST_INIT(__open_at_start);

/* load the list of all open file descriptors into the given list */
static void __load_open_fds(struct list *l)
{
	DIR *fds;
	struct dirent *fd;
	struct open_fd *newentry;
	int skip;
	int openfd;

	fds = opendir("/proc/self/fd");
	skip = dirfd(fds);

	for (fd = readdir(fds); fd != NULL; fd = readdir(fds)) {
		openfd = atoi(fd->d_name);
		if (openfd == skip)
			continue;

		newentry = malloc(sizeof(*newentry));
		assert(newentry != NULL);
		newentry->fd = openfd;
		list_pushfront(l, &newentry->le);
	}

	closedir(fds);
}

/* check if an fd is present in a list */
static int __fd_is_present(struct list *fds, int fd)
{
	struct list_elem *e;
	struct open_fd *fd_in_set;

	list_foreach(e, fds) {
		fd_in_set = containerof(e, struct open_fd, le);
		if (fd == fd_in_set->fd)
			return 1;
	}
	return 0;
}

/* free all tracked fds from a set */
static void __free_fd_set(struct list *fds)
{
	struct list_elem *e;
	struct open_fd *fd;

	while (!list_isempty(fds)) {
		e = list_popfront(fds);
		fd = containerof(e, struct open_fd, le);
		free(fd);
	}
}

static void __check_open_fds(void)
{
	LIST_INIT(open_at_end);
	struct list_elem *e;
	struct open_fd *fd;

	/* get list of open file descriptors at the end of execution */
	__load_open_fds(&open_at_end);

	list_foreach(e, &open_at_end) {
		fd = containerof(e, struct open_fd, le);
		if (!__fd_is_present(&__open_at_start, fd->fd))
			fprintf(stderr, "leaked file descriptor: %d\n", fd->fd);
	}

	/* free existing fd sets */
	__free_fd_set(&open_at_end);
	__free_fd_set(&__open_at_start);
}

static void __attribute__ ((constructor)) __start_open_fds(void)
{
	int rc;

	/* store list of all open file descriptors at start */
	__load_open_fds(&__open_at_start);

	/* at exit, check that no additional descriptors are open */
	rc = atexit(__check_open_fds);
	assert(rc == 0);
}

