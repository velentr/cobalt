/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "dstring.h"
#include "find.h"
#include "util.h"

static const char *find_path(struct dstring *path)
{
	struct stat st;
	dev_t dev;
	ino_t ino;
	int rc;

	memset(&st, 0, sizeof(st));
	do {
		dev = st.st_dev;
		ino = st.st_ino;
		if (co_exists(dstr(path)))
			return dstr(path);

		rc = stat(dstr(path), &st);
		if (rc != 0)
			return NULL;

		rc = dstrcat(path, "/..");
		if (rc != 0)
			return NULL;
	} while (st.st_ino != ino || st.st_dev != dev);

	return NULL;
}

struct cobalt *find_cobalt(const char *start)
{
	struct dstring pathmem;
	const char *env;
	const char *path;
	struct cobalt *co;
	int rc;

	if (start != NULL)
		rc = dstrcpy(&pathmem, start);
	else if ((env = getenv("COBALT_PATH")) != NULL)
		rc = dstrcpy(&pathmem, env);
	else
		rc = dstrcpy(&pathmem, ".");

	if (rc != 0) {
		eprint("%s\n", strerror(rc));
		return NULL;
	}

	path = find_path(&pathmem);
	if (path == NULL) {
		dstrclr(&pathmem);
		eprint("cannot find cobalt database\n");
		return NULL;
	}

	co = co_open(path, &rc);
	if (co == NULL)
		eprint("cannot open cobalt database at %s: %s\n", path,
				strerror(rc));
	dstrclr(&pathmem);

	return co;
}

