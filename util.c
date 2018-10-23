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
	close(fd);

	return 0;
}

