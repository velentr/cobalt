/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <assert.h>
#include <errno.h>
#include <ftw.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "dstring.h"
#include "struct.h"
#include "util.h"

static int co_gc_fn_strict(const char *path, const struct stat *st UNUSED,
		int type, struct FTW *ftwbuf UNUSED)
{
	int rc;

	if (type == FTW_DP)
		rc = rmdir(path);
	else
		rc = unlink(path);

	if (rc != 0)
		return errno;
	else
		return 0;
}

static int co_gc_fn(const char *path, const struct stat *st, int type,
		struct FTW *ftwbuf)
{
	(void)co_gc_fn_strict(path, st, type, ftwbuf);
	return 0;
}

int co_gc(struct cobalt *co, uint32_t flags)
{
	struct dstring root;
	int rc;

	dstrempty(&root);

	rc = dstrcatl(&root, dstr(&co->path), dstrlen(&co->path));
	if (rc != CO_ENOERR)
		return rc;

	rc = dstrcat(&root, "/gc/");
	if (rc != CO_ENOERR) {
		dstrclr(&root);
		return rc;
	}

	if (flags & CO_GC_STRICT)
		rc = nftw(dstr(&root), co_gc_fn_strict, 32,
				FTW_DEPTH | FTW_PHYS);
	else
		rc = nftw(dstr(&root), co_gc_fn, 32, FTW_DEPTH | FTW_PHYS);

	if (rc == -1) {
		co->err = errno;
		rc = errno;
	} else if (rc != CO_ENOERR) {
		co->err = rc;
	}

	return rc;
}
