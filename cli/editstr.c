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

static void editstr_init(struct editstr *es)
{
	/* initialize the editstr to default values to check during cleanup */
	es->fname[0] = '\0';
	es->data = MAP_FAILED;
	es->len = 0;
	es->fd = -1;
}

static int editstr_map(const char *fname, struct editstr *es)
{
	struct stat st;
	int rc;

	es->fd = open(fname, O_RDONLY);
	if (es->fd == -1)
		return errno;

	/* Get the file size for mmap() */
	rc = fstat(es->fd, &st);
	if (rc == -1)
		return errno;
	es->len = st.st_size;
	if (es->len == 0)
		return ECANCELED;

	es->data = mmap(NULL, es->len, PROT_READ, MAP_PRIVATE, es->fd, 0);
	if (es->data == MAP_FAILED)
		return errno;

	return 0;
}

static void editstr_destroy(struct editstr *es)
{
	if (es->fname[0] != '\0')
		unlink(es->fname);
	if (es->data != MAP_FAILED)
		munmap(es->data, es->len);
	if (es->fd != -1)
		close(es->fd);
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
		rc = errno;
		goto error_destroy;
	}
	strcpy(es.fname, fname);

	/* write template to the file before opening in editor */
	if (template != NULL && len > 0) {
		rc = write(fd, template, len);
		if (rc < 0) {
			rc = errno;
			close(fd);
			goto error_destroy;
		}
	}

	/* On some systems, the editor opens using CoW, so we need to re-open in
	 * this function in order to mmap() the updated file
	 */
	close(fd);

	rc = system(cmd);
	if (rc != 0) {
		rc = EINVAL;	/* TODO get a better error code here */
		goto error_destroy;
	}

	rc = editstr_map(fname, &es);
	if (rc != 0)
		goto error_destroy;

	rc = editstr_write(s, &es);
error_destroy:
	editstr_destroy(&es);
	return rc;
}

