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

#include "db.h"
#include "dstring.h"
#include "list.h"
#include "random.h"
#include "struct.h"
#include "util.h"

#define CO_SID_LEN sizeof("XXXXXXXX/")

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
	const struct co_db_op create[] = {
		OP_SCAT(0, dstr(&co->path), dstrlen(&co->path)),
		OP_DCREAT(0),
	};

	return co_db_run(&co->db, create, lengthof(create));
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
		*err = ENOMEM;
		return NULL;
	}

	co_rng_init(&co->rng);

	rc = dstrcpy(&co->path, path);
	if (rc == -1) {
		free(co);
		*err = ENOMEM;
		return NULL;
	}

	rc = dstrcat(&co->path, "/.cobalt");
	if (rc == -1) {
		dstrclr(&co->path);
		free(co);
		*err = ENOMEM;
		return NULL;
	}


	co_db_init(&co->db);

	return co;
}

void co_free(struct cobalt *co)
{
	co_db_free(&co->db);
	dstrclr(&co->path);
	free(co);
}

static int co_do_add(struct cobalt *co, const char *sid, const char *data,
		size_t len, const char *board)
{
	const struct co_db_op add[] = {
		OP_SCAT(0, dstr(&co->path), dstrlen(&co->path)),
		OP_SCPY(3, 0),
		MACRO_SCATL(0, "/obj/"),
		OP_DEXIST(0),
		OP_SCAT(0, sid, CO_SID_LEN - 1),
		OP_DCREAT(0),
		OP_SCPY(1, 0),
		MACRO_SCATL(0, "data"),
		OP_FCREAT(0, data, len),
		MACRO_SCATL(1, "attr/"),
		OP_DCREAT(1),
		MACRO_SCATL(1, "board"),
		MACRO_SCATL(2, "../../../attr/board/"),
		MACRO_SCATL(2, board),
		OP_LCREAT(2, 1),

		MACRO_SCATL(3, "/attr/"),
		OP_DEXIST(3),
		MACRO_SCATL(3, "board/"),
		OP_DEXIST(3),
		MACRO_SCATL(3, board),
		MACRO_SCATL(3, "/"),
		OP_DEXIST(3),
		OP_SCAT(3, sid, CO_SID_LEN - 2),
		MACRO_SCATL(4, "../../../obj/"),
		OP_SCAT(4, sid, CO_SID_LEN - 2),
		OP_LCREAT(4, 3),
	};
	return co_db_run(&co->db, add, lengthof(add));
}

uint32_t co_add(struct cobalt *co, const char *data, size_t len,
		const char *board)
{
	char sid[CO_SID_LEN];
	uint32_t id;
	int rc;

	id = co_getrandomid(&co->rng, &co->err);
	if (id == 0)
		return 0;
	snprintf(sid, sizeof(sid), "%08x/", id);

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
	const struct co_db_op mod_data[] = {
		OP_SCAT(0, dstr(&co->path), dstrlen(&co->path)),
		MACRO_SCATL(0, "/obj/"),
		OP_SCAT(0, sid, CO_SID_LEN - 1),
		OP_SCPY(1, 0),
		MACRO_SCATL(0, "tmp"),
		MACRO_SCATL(1, "data"),

		OP_FCREAT(0, data, len),
		OP_FRENAM(0, 1),
	};
	return co_db_run(&co->db, mod_data, lengthof(mod_data));
}

int co_mod_data(struct cobalt *co, uint32_t id, const char *data, size_t len)
{
	char sid[CO_SID_LEN];
	int rc;

	snprintf(sid, sizeof(sid), "%08x/", id);

	rc = co_do_mod_data(co, sid, data, len);
	co->err = rc;
	return rc;
}

static int co_do_mod_attr(struct cobalt *co, const char *sid, const char *name,
		const char *newval)
{
	const struct co_db_op mod_attr[] = {
		OP_SCAT(0, dstr(&co->path), dstrlen(&co->path)),
		OP_SCPY(1, 0),
		MACRO_SCATL(0, "/attr/"),
		MACRO_SCATL(0, name),
		MACRO_SCATL(0, "/"),
		OP_DEXIST(0),
		MACRO_SCATL(0, newval),
		MACRO_SCATL(0, "/"),
		OP_DEXIST(0),
		OP_SCAT(0, sid, CO_SID_LEN - 2),

		MACRO_SCATL(1, "/obj/"),
		OP_SCAT(1, sid, CO_SID_LEN - 1),
		OP_SCPY(3, 1),
		MACRO_SCATL(1, "attr/"),
		MACRO_SCATL(1, name),
		OP_SCPY(4, 1),
		MACRO_SCATL(1, "/"),
		OP_SCAT(1, sid, CO_SID_LEN - 2),

		MACRO_SCATL(2, "../../../obj/"),
		OP_SCAT(2, sid, CO_SID_LEN - 2),

		MACRO_SCATL(3, "tmp"),

		MACRO_SCATL(5, "../../../attr/"),
		MACRO_SCATL(5, name),
		MACRO_SCATL(5, "/"),
		MACRO_SCATL(5, newval),

		OP_LCREAT(2, 0),
		OP_LDEL(2, 1),
		OP_LCREAT(5, 3),
		OP_FRENAM(3, 4),

	};
	return co_db_run(&co->db, mod_attr, lengthof(mod_attr));
}

int co_mod_attr(struct cobalt *co, uint32_t id, const char *name,
		const char *newval)
{
	char sid[CO_SID_LEN];
	int rc;

	snprintf(sid, sizeof(sid), "%08x/", id);

	rc = co_do_mod_attr(co, sid, name, newval);
	co->err = rc;
	return rc;
}

static int co_do_delete(struct cobalt *co, const char *sid)
{
	struct co_db_op delete[] = {
		OP_SCAT(0, dstr(&co->path), dstrlen(&co->path)),
		OP_SCPY(1, 0),

		MACRO_SCATL(0, "/obj/"),
		OP_SCAT(0, sid, CO_SID_LEN - 1),
		OP_SCPY(3, 0),
		MACRO_SCATL(0, "attr/board/"),
		OP_SCAT(0, sid, CO_SID_LEN - 2),
		MACRO_SCATL(2, "../../../obj/"),
		OP_SCAT(2, sid, CO_SID_LEN - 2),
		OP_LDEL(2, 0),

		MACRO_SCATL(1, "/gc/"),
		OP_DEXIST(1),
		OP_SCAT(1, sid, CO_SID_LEN - 1),
		OP_DCREAT(1),
		OP_FRENAM(3, 1),
	};
	return co_db_run(&co->db, delete, lengthof(delete));
}

int co_delete(struct cobalt *co, uint32_t id, uint32_t flags)
{
	char sid[CO_SID_LEN];
	int rc;
	uint32_t gcflags;

	snprintf(sid, sizeof(sid), "%08x/", id);

	rc = co_do_delete(co, sid);
	if (rc != CO_ENOERR) {
		co->err = rc;
		return rc;
	}

	if (!(flags & CO_DEL_NGC)) {
		gcflags = (flags & CO_DEL_STRICTGC) ? CO_GC_STRICT : 0;
		rc = co_gc(co, gcflags);
	}

	return rc;
}

int co_get_task(struct cobalt *co, uint32_t id, struct cobalt_query **q)
{
	int rc;
	struct list_elem *result;
	struct co_db_op get_id[] = {
		OP_SCAT(0, dstr(&co->path), dstrlen(&co->path)),
		MACRO_SCATL(0, "/obj/"),
		OP_QILD(0),
		OP_QDRD(id),
		OP_QMAP,
	};
	rc = co_db_run(&co->db, get_id, lengthof(get_id));
	if (rc == CO_ENOERR) {
		assert(!list_isempty(&co->db.query));
		result = list_popfront(&co->db.query);
		*q = containerof(result, struct cobalt_query, le);
		return CO_ENOERR;
	} else {
		co->err = rc;
		return rc;
	}
}

int co_get_board(struct cobalt *co, const char *board, struct cobalt_query **q)
{
	int rc;
	struct list_elem *result;
	struct co_db_op get_board[] = {
		OP_SCAT(0, dstr(&co->path), dstrlen(&co->path)),
		MACRO_SCATL(0, "/attr/board/"),
		MACRO_SCATL(0, board),
		OP_QILD(0),
		OP_QIRD,
		OP_QMAP,
	};

	rc = co_db_run(&co->db, get_board, lengthof(get_board));
	if (rc == CO_ENOERR) {
		if (list_isempty(&co->db.query)) {
			*q = NULL;
		} else {
			result = list_popfront(&co->db.query);
			*q = containerof(result, struct cobalt_query, le);
		}
		return CO_ENOERR;
	} else {
		co->err = rc;
		return rc;
	}
}

struct cobalt_query *co_query_getnext(struct cobalt *co, struct cobalt_query *q)
{
	struct cobalt_query *result;
	struct list_elem *first;

	co_db_query_free(q);

	if (list_isempty(&co->db.query)) {
		result = NULL;
	} else {
		first = list_popfront(&co->db.query);
		result = containerof(first, struct cobalt_query, le);
	}

	return result;
}

uint32_t co_query_getid(struct cobalt_query *q)
{
	return q->id;
}

const char *co_query_getboard(struct cobalt_query *q)
{
	/* return dstr(&q->board); */
	(void)q;
	return NULL;
}

void co_query_getdata(struct cobalt_query *q, const char **data, size_t *len)
{
	if (data != NULL)
		*data = q->data;
	if (len != NULL)
		*len = q->len;
}

