/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "editstr.h"
#include "util.h"

static void editstr_init(struct editstr *es)
{
	/* initialize the editstr to default values to check during cleanup */
	es->fname[0] = '\0';
	es->data = MAP_FAILED;
	es->len = 0;
}

static void editstr_destroy(struct editstr *es)
{
	unlink(es->fname);
	munmap((void *)es->data, es->len);
}

int editstr_create(struct dstring *s, const char *template, size_t len)
{
	char cmd[] = "$EDITOR " EDITSTR_TMPFILE;
	struct editstr es;
	char *fname;
	int fd;
	int rc;

	editstr_init(&es);

	/* Since the string should match the above literal, there will always be
	 * a '/' character at the start of the filename.
	 */
	fname = strchr(cmd, '/');
	assert(fname != NULL);

	fd = mkstemp(fname);
	if (fd == -1) {
		return errno;
	}
	strcpy(es.fname, fname);

	/* write template to the file before opening in editor */
	if (template != NULL && len > 0) {
		rc = write(fd, template, len);
		if (rc < 0) {
			rc = errno;
			close(fd);
			return rc;
		}
	}

	/* On some systems, the editor opens using CoW, so we need to re-open in
	 * this function in order to mmap() the updated file
	 */
	close(fd);

	rc = system(cmd);
	if (rc != 0) {
		return EINVAL;
	}

	rc = fmap(fname, &es.data, &es.len);
	if (rc != 0)
		return rc;

	rc = editstr_write(s, &es);
	editstr_destroy(&es);

	return rc;
}

