/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(CO_VALGRIND)
#include <valgrind/valgrind.h>
#else
#define VALGRIND_MALLOCLIKE_BLOCK(addr, sizeB, rzB, is_zeroed)	(void)0
#define VALGRIND_FREELIKE_BLOCK(addr, rzB)			(void)0
#endif

#include "util.h"

int fmap(const char *fname, const char **buf, size_t *len)
{
	struct stat st;
	int fd;
	int rc;

	assert(fname != NULL);
	assert(buf != NULL);
	assert(len != NULL);

	fd = open(fname, O_RDONLY);
	if (fd == -1)
		return errno;

	rc = fstat(fd, &st);
	if (rc == -1) {
		rc = errno;
		close(fd);
		return rc;
	}

	*len = st.st_size;
	if (*len == 0) {
		rc = ECANCELED;
		close(fd);
		return rc;
	}

	*buf = mmap(NULL, *len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (*buf == MAP_FAILED) {
		rc = errno;
		close(fd);
		return rc;
	}

	/* valgrind cannot track mmap()'d blocks, because there would be way too
	 * many false positives mapped by the dynamic linker that the OS must
	 * clean up; instead, we must manually tell valgrind to track an
	 * mmap()'d block
	 */
	VALGRIND_MALLOCLIKE_BLOCK(*buf, *len, 0, 1);

	close(fd);

	return 0;
}

void funmap(const char *buf, size_t len)
{
	VALGRIND_FREELIKE_BLOCK(buf, 0);
	munmap((void *)buf, len);
}

