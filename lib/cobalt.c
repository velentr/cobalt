/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>
#include <cobalt/cobalt_error.h>

#include "dstring.h"
#include "fsvm.h"
#include "list.h"
#include "random.h"
#include "struct.h"
#include "util.h"

#define CO_ID_STRLEN	8

#define CO_MAP_REG	0
#define CO_BOARD_REG	1

/* validate that 'attr' is valid for an attribute name or value */
static int co_validate_attr(const char *attr)
{
	const char *p;
	size_t len;

	if (*attr == '.')
		return EINVAL;

	for (p = attr, len = 0;; p++) {
		switch (*p) {
		case '\0':
			return len == 0 ? EINVAL : CO_ENOERR;
		case '/':
			return EINVAL;
		default:
			len++;
			continue;
		}
	}
}

int co_exists(const char *_path)
{
	struct dstring path;
	struct stat st;
	int rc;

	rc = dstrcpy(&path, _path);
	if (rc != 0)
		return 0;
	rc = dstrcat(&path, "/.cobalt");
	if (rc != 0)
		return 0;

	rc = stat(dstr(&path), &st);
	if (rc == 0 && S_ISDIR(st.st_mode))
		return 1;
	else
		return 0;
}

void co_version(int *major, int *minor, int *patch)
{
	if (major != NULL)
		*major = CO_MAJOR;

	if (minor != NULL)
		*minor = CO_MINOR;

	if (patch != NULL)
		*patch = CO_PATCH;
}

static int co_create(struct cobalt *co)
{
	struct fsvm vm;
	const struct fsvm_op create[] = {
		OP_ACAT(0, 0),
		OP_DMK(0),
	};
	int rc;

	fsvm_init(&vm);
	fsvm_ldarg(&vm, 0, dstr(&co->path), dstrlen(&co->path));

	rc = fsvm_run(&vm, create, lengthof(create));

	fsvm_clear(&vm);

	return rc;
}

struct cobalt *co_init(const char *path, int *err)
{
	struct cobalt *co;
	int rc;

	co = co_open(path, err);
	if (co == NULL)
		return NULL;

	rc = co_create(co);
	if (rc != CO_ENOERR) {
		co_free(co);
		*err = rc;
		return NULL;
	} else {
		return co;
	}
}

struct cobalt *co_open(const char *path, int *err)
{
	struct cobalt *co;
	int rc;

	co = malloc(sizeof(*co));
	if (co == NULL) {
		*err = errno;
		return NULL;
	}

	co_rng_init(&co->rng);

	rc = dstrcpy(&co->path, path);
	if (rc == -1) {
		free(co);
		*err = rc;
		return NULL;
	}

	rc = dstrcat(&co->path, "/.cobalt");
	if (rc == -1) {
		dstrclr(&co->path);
		free(co);
		*err = rc;
		return NULL;
	}

	list_init(&co->globs);

	return co;
}

void co_free(struct cobalt *co)
{
	struct fsvm_glob *g;

	if (co != NULL) {
		while (!list_isempty(&co->globs)) {
			g = to_fsvm_glob(list_popfront(&co->globs));
			fsvm_globfree(g);
		}

		dstrclr(&co->path);
		free(co);
	}
}

static int co_do_add(struct cobalt *co, const char *sid, const char *data,
		size_t len, const char *board)
{
	struct fsvm vm;
	const struct fsvm_op add[] = {
		OP_ACAT(0, 0),
		OP_RCAT(3, 0),
		OP_ACAT(0, 1),
		OP_DEX(0),
		OP_ACAT(0, 2),
		OP_DMK(0),
		OP_RCAT(1, 0),
		OP_ACAT(0, 3),
		OP_FMK(0, 4),
		OP_ACAT(1, 5),
		OP_DMK(1),
		OP_ACAT(1, 6),
		OP_ACAT(2, 7),
		OP_ACAT(2, 8),
		OP_LNK(2, 1),

		OP_ACAT(3, 9),
		OP_DEX(3),
		OP_ACAT(3, 10),
		OP_DEX(3),
		OP_ACAT(3, 8),
		OP_ACAT(3, 11),
		OP_DEX(3),
		OP_ACAT(3, 12),
		OP_ACAT(4, 13),
		OP_ACAT(4, 12),
		OP_LNK(4, 3),
	};
	int rc;

	fsvm_init(&vm);
	fsvm_ldarg(&vm,  0, dstr(&co->path), dstrlen(&co->path));
	fsvm_ldarg(&vm,  1, "/obj/", strlen("/obj/"));
	fsvm_ldarg(&vm,  2, sid, CO_ID_STRLEN + 1);
	fsvm_ldarg(&vm,  3, "data", strlen("data"));
	fsvm_ldarg(&vm,  4, data, len);
	fsvm_ldarg(&vm,  5, "attr/", strlen("attr/"));
	fsvm_ldarg(&vm,  6, "board", strlen("board"));
	fsvm_ldarg(&vm,  7, "../../../attr/board/",
			strlen("../../../attr/board/"));
	fsvm_ldarg(&vm,  8, board, strlen(board));
	fsvm_ldarg(&vm,  9, "/attr/", strlen("/attr/"));
	fsvm_ldarg(&vm, 10, "board/", strlen("board/"));
	fsvm_ldarg(&vm, 11, "/", strlen("/"));
	fsvm_ldarg(&vm, 12, sid, CO_ID_STRLEN);
	fsvm_ldarg(&vm, 13, "../../../obj/", strlen("../../../obj/"));

	rc = fsvm_run(&vm, add, lengthof(add));

	fsvm_clear(&vm);
	return rc;
}

uint32_t co_add(struct cobalt *co, const char *data, size_t len,
		const char *board)
{
	char sid[CO_ID_STRLEN + 2];
	uint32_t id;
	int rc;

	/* make sure the passed in board name is valid */
	rc = co_validate_attr(board);
	if (rc != CO_ENOERR) {
		co->err = rc;
		return 0;
	}

	id = co_getrandomid(&co->rng, &co->err);
	if (id == 0)
		return 0;
	snprintf(sid, lengthof(sid), "%08x/", id);

	rc = co_do_add(co, sid, data, len, board);
	if (rc != CO_ENOERR) {
		co->err = rc;
		return 0;
	} else {
		return id;
	}
}

static int co_do_mod_data(struct cobalt *co, const char *sid, const char *data,
		size_t len)
{
	struct fsvm vm;
	const struct fsvm_op mod_data[] = {
		OP_ACAT(0, 0),	// r0 <- co->path
		OP_ACAT(0, 1),	// r0 <- "/obj/"
		OP_ACAT(0, 2),	// r0 <- $sid
		OP_RCAT(1, 0),	// r1 <- r0
		OP_ACAT(0, 3),	// r0 <- "tmp"
		OP_ACAT(1, 4),	// r1 <- "data"
		OP_FMK(0, 5),	// fmk r0 $data
		OP_FMV(0, 1),	// fmv r0 r1
	};
	int rc;

	fsvm_init(&vm);
	fsvm_ldarg(&vm, 0, dstr(&co->path), dstrlen(&co->path));
	fsvm_ldarg(&vm, 1, "/obj/", strlen("/obj/"));
	fsvm_ldarg(&vm, 2, sid, CO_ID_STRLEN + 1);
	fsvm_ldarg(&vm, 3, "tmp", strlen("tmp"));
	fsvm_ldarg(&vm, 4, "data", strlen("data"));
	fsvm_ldarg(&vm, 5, data, len);

	rc = fsvm_run(&vm, mod_data, lengthof(mod_data));

	fsvm_clear(&vm);
	return rc;
}

int co_mod_data(struct cobalt *co, uint32_t id, const char *data, size_t len)
{
	char sid[CO_ID_STRLEN + 2];
	int rc;

	/* check for an invalid id */
	if (id == 0) {
		co->err = EINVAL;
		return EINVAL;
	}

	snprintf(sid, lengthof(sid), "%08x/", id);

	rc = co_do_mod_data(co, sid, data, len);
	co->err = rc;
	return rc;
}

static int co_do_mod_attr(struct cobalt *co, const char *sid, const char *name,
		const char *newval)
{
	struct fsvm vm;
	const struct fsvm_op mod_attr[] = {
		OP_ACAT(0, 0),
		OP_RCAT(1, 0),
		OP_ACAT(0, 1),
		OP_ACAT(0, 2),
		OP_ACAT(0, 3),
		OP_DEX(0),
		OP_ACAT(0, 4),
		OP_ACAT(0, 3),
		OP_DEX(0),
		OP_ACAT(0, 5),

		OP_ACAT(1, 7),
		OP_ACAT(1, 5),
		OP_RCAT(3, 1),
		OP_ACAT(1, 1),
		OP_ACAT(1, 2),
		OP_RCAT(4, 1),
		OP_ACAT(1, 6),
		OP_ACAT(1, 5),

		OP_ACAT(2, 8),
		OP_ACAT(2, 5),
		OP_ACAT(3, 9),

		OP_ACAT(5, 10),
		OP_ACAT(5, 2),
		OP_ACAT(5, 6),
		OP_ACAT(5, 4),

		OP_LNK(2, 0),
		OP_DLNK(2, 1),
		OP_LNK(5, 3),
		OP_FMV(3, 4),
	};
	int rc;

	fsvm_init(&vm);
	fsvm_ldarg(&vm,  0, dstr(&co->path), dstrlen(&co->path));
	fsvm_ldarg(&vm,  1, "/attr/", strlen("/attr/"));
	fsvm_ldarg(&vm,  2, name, strlen(name));
	fsvm_ldarg(&vm,  3, "/", strlen("/"));
	fsvm_ldarg(&vm,  4, newval, strlen(newval));
	fsvm_ldarg(&vm,  5, sid, CO_ID_STRLEN);
	fsvm_ldarg(&vm,  6, "/", strlen("/"));
	fsvm_ldarg(&vm,  7, "/obj/", strlen("/obj/"));
	fsvm_ldarg(&vm,  8, "../../../obj/", strlen("../../../obj/"));
	fsvm_ldarg(&vm,  9, "/tmp", strlen("/tmp"));
	fsvm_ldarg(&vm, 10, "../../../attr/", strlen("../../../attr/"));

	rc = fsvm_run(&vm, mod_attr, lengthof(mod_attr));

	fsvm_clear(&vm);

	return rc;
}

int co_mod_attr(struct cobalt *co, uint32_t id, const char *name,
		const char *newval)
{
	char sid[CO_ID_STRLEN + 2];
	int rc;

	/* validate user input */
	rc = co_validate_attr(name);
	if (rc != CO_ENOERR) {
		co->err = rc;
		return rc;
	}
	rc = co_validate_attr(newval);
	if (rc != CO_ENOERR) {
		co->err = rc;
		return rc;
	}
	if (id == 0) {
		co->err = EINVAL;
		return EINVAL;
	}

	snprintf(sid, lengthof(sid), "%08x/", id);

	rc = co_do_mod_attr(co, sid, name, newval);
	co->err = rc;
	return rc;
}

int co_delete(struct cobalt *co, uint32_t id, uint32_t flags)
{
	struct fsvm vm;
	struct fsvm_op delete[] = {
		OP_ACAT(0, 0),
		OP_RCAT(1, 0),

		OP_ACAT(0, 1),
		OP_ACAT(0, 3),
		OP_RCAT(3, 0),
		OP_ACAT(0, 2),
		OP_ACAT(0, 4),
		OP_ACAT(2, 5),
		OP_ACAT(2, 4),
		OP_DLNK(2, 0),

		OP_ACAT(1, 6),
		OP_DEX(1),
		OP_FMV(3, 1),
	};
	char sid[CO_ID_STRLEN + 2];
	int rc;
	uint32_t gcflags;

	if (id == 0) {
		co->err = EINVAL;
		return EINVAL;
	}

	snprintf(sid, lengthof(sid), "%08x/", id);

	fsvm_init(&vm);
	fsvm_ldarg(&vm, 0, dstr(&co->path), dstrlen(&co->path));
	fsvm_ldarg(&vm, 1, "/obj/", strlen("/obj/"));
	fsvm_ldarg(&vm, 2, "attr/board/", strlen("attr/board/"));
	fsvm_ldarg(&vm, 3, sid, CO_ID_STRLEN + 1);
	fsvm_ldarg(&vm, 4, sid, CO_ID_STRLEN);
	fsvm_ldarg(&vm, 5, "../../../obj/", strlen("../../../obj/"));
	fsvm_ldarg(&vm, 6, "/gc/", strlen("/gc/"));

	rc = fsvm_run(&vm, delete, lengthof(delete));
	if (rc != CO_ENOERR) {
		co->err = rc;
	} else if (!(flags & CO_DEL_NGC)) {
		gcflags = (flags & CO_DEL_STRICTGC) ? CO_GC_STRICT : 0;
		rc = co_gc(co, gcflags);
	}

	fsvm_clear(&vm);
	return rc;
}

static struct cobalt_query *co_get_first(struct cobalt *co)
{
	struct list_elem *le;
	struct fsvm_glob *g;

	assert(co != NULL);

	if (list_isempty(&co->globs)) {
		return NULL;
	} else {
		le = list_head(&co->globs);
		g = to_fsvm_glob(le);
		return glob_to_query(g);
	}
}

int co_get_task(struct cobalt *co, uint32_t id, struct cobalt_query **q)
{
	struct fsvm vm;
	struct fsvm_op get_board[] = {
		OP_ACAT(0, 0),
		OP_ACAT(0, 1),
		OP_ACAT(2, 2),
		OP_ACAT(3, 3),
		OP_ACAT(4, 4),
		OP_LD(0),
		OP_RD(4),
		OP_MAP(CO_MAP_REG, 2),
		OP_RDLNK(CO_BOARD_REG, 3),
	};
	char sid[CO_ID_STRLEN + 1];
	int rc;

	if (id == 0) {
		co->err = EINVAL;
		return EINVAL;
	}

	rc = snprintf(sid, lengthof(sid), "%08x", id);
	assert(rc == CO_ID_STRLEN);

	fsvm_init(&vm);
	fsvm_ldarg(&vm, 0, dstr(&co->path), dstrlen(&co->path));
	fsvm_ldarg(&vm, 1, "/obj", strlen("/obj"));
	fsvm_ldarg(&vm, 2, "data", strlen("data"));
	fsvm_ldarg(&vm, 3, "attr/board", strlen("attr/board"));
	fsvm_ldarg(&vm, 4, sid, CO_ID_STRLEN);

	rc = fsvm_run(&vm, get_board, lengthof(get_board));
	if (rc == CO_ENOERR) {
		fsvm_globs(&vm, &co->globs);
		*q = co_get_first(co);
	} else {
		co->err = rc;
	}

	fsvm_clear(&vm);
	return rc;
}

int co_get_board(struct cobalt *co, const char *board, struct cobalt_query **q)
{
	struct fsvm vm;
	struct fsvm_op get_board[] = {
		OP_ACAT(0, 0),
		OP_ACAT(0, 1),
		OP_ACAT(0, 4),
		OP_ACAT(2, 2),
		OP_ACAT(3, 3),
		OP_LD(0),
		OP_GLOB,
		OP_MAP(CO_MAP_REG, 2),
		OP_RDLNK(CO_BOARD_REG, 3),
	};
	int rc;

	rc = co_validate_attr(board);
	if (rc != CO_ENOERR) {
		co->err = rc;
		return rc;
	}

	fsvm_init(&vm);
	fsvm_ldarg(&vm, 0, dstr(&co->path), dstrlen(&co->path));
	fsvm_ldarg(&vm, 1, "/attr/board/", strlen("/attr/board/"));
	fsvm_ldarg(&vm, 2, "data", strlen("data"));
	fsvm_ldarg(&vm, 3, "attr/board", strlen("attr/board"));
	fsvm_ldarg(&vm, 4, board, strlen(board));

	rc = fsvm_run(&vm, get_board, lengthof(get_board));
	if (rc == CO_ENOERR) {
		fsvm_globs(&vm, &co->globs);
		*q = co_get_first(co);
	} else {
		co->err = rc;
	}

	fsvm_clear(&vm);
	return rc;
}

int co_get_all(struct cobalt *co, struct cobalt_query **q)
{
	struct fsvm vm;
	struct fsvm_op get_board[] = {
		OP_ACAT(0, 0),
		OP_ACAT(0, 1),
		OP_ACAT(2, 2),
		OP_ACAT(3, 3),
		OP_LD(0),
		OP_GLOB,
		OP_MAP(CO_MAP_REG, 2),
		OP_RDLNK(CO_BOARD_REG, 3),
	};
	int rc;

	fsvm_init(&vm);
	fsvm_ldarg(&vm, 0, dstr(&co->path), dstrlen(&co->path));
	fsvm_ldarg(&vm, 1, "/obj", strlen("/obj"));
	fsvm_ldarg(&vm, 2, "data", strlen("data"));
	fsvm_ldarg(&vm, 3, "attr/board", strlen("attr/board"));

	rc = fsvm_run(&vm, get_board, lengthof(get_board));
	if (rc == CO_ENOERR) {
		fsvm_globs(&vm, &co->globs);
		*q = co_get_first(co);
	} else {
		co->err = rc;
	}

	fsvm_clear(&vm);
	return rc;
}

struct cobalt_query *co_query_getnext(struct cobalt *co,
		UNUSED struct cobalt_query *q)
{
	struct fsvm_glob *g;
	struct list_elem *first;

	assert(!list_isempty(&co->globs));

	first = list_popfront(&co->globs);
	g = to_fsvm_glob(first);
	assert(g == query_to_glob(q));
	fsvm_globfree(g);

	return co_get_first(co);
}

uint32_t co_query_getid(struct cobalt_query *q)
{
	struct fsvm_glob *g;
	char *endp;
	unsigned long id;

	g = query_to_glob(q);
	id = strtoul(g->name, &endp, 16);
	if (id == 0 || *endp != '\0' || endp - g->name != CO_ID_STRLEN)
		return CO_ECORRUPT;

	return id;
}

const char *co_query_getboard(struct cobalt_query *q)
{
	struct fsvm_glob *g;

	g = query_to_glob(q);

	// TODO don't hard-code this
	return dstr(&g->reg[CO_BOARD_REG]) + 20;
}

void co_query_getdata(struct cobalt_query *q, const char **data, size_t *len)
{
	struct fsvm_glob *g;

	g = query_to_glob(q);

	if (data != NULL)
		*data = dstr(&g->reg[CO_MAP_REG]);
	if (len != NULL)
		*len = dstrlen(&g->reg[CO_MAP_REG]);
}

