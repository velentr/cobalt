/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "argparse.h"
#include "editstr.h"
#include "modules.h"
#include "util.h"

struct arg delete;
struct arg edit;
struct arg nogc;
struct arg destination = {
	.name = "destination",
	.desc = "board to which the task is moved",
	.type = ARG_BOARD,
	.exclude = { &delete, &nogc, &edit, NULL }
};
struct arg delete = {
	.name = "delete",
	.desc = "delete the specified task",
	.type = ARG_BOOL,
	.boolean = {
		.lmatch = "delete",
		.smatch = 'd',
	},
	.exclude = { &edit, &destination, NULL }
};
struct arg nogc = {
	.name = "no-gc",
	.desc = "do not run garbage collection after deletion",
	.type = ARG_BOOL,
	.boolean = {
		.lmatch = "no-gc",
		.smatch = 'n',
	},
	.exclude = { &edit, &destination, NULL }
};
struct arg edit = {
	.name = "edit",
	.desc = "edit the task's data using $EDITOR",
	.type = ARG_BOOL,
	.boolean = {
		.lmatch = "edit",
		.smatch = 'e',
	},
	.exclude = { &delete, &nogc, &destination, NULL }
};

struct arg id = {
	.name = "id",
	.desc = "numeric id of the task to modify",
	.type = ARG_ID,
	.exclude = { NULL }
};

static int modify_do_delete(struct cobalt *co, uint32_t id)
{
	int rc;

	assert(id != 0);

	if (nogc.valid && nogc.boolean.value)
		rc = co_delete(co, id, CO_DEL_NGC);
	else
		rc = co_delete(co, id, 0);

	if (rc != CO_ENOERR) {
		eprint("cannot delete task %08x: %s\n", id, co_strerror(co));
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}

static int modify_do_move(struct cobalt *co, uint32_t id, const char *board)
{
	int rc;

	assert(id != 0);

	if (board == NULL) {
		eprint("board required to move task\n");
		return EXIT_FAILURE;
	}

	rc = co_mod_attr(co, id, "board", board);
	if (rc != CO_ENOERR) {
		eprint("cannot move %08x to board '%s': %s\n", id, board,
				co_strerror(co));
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}

static int modify_do_edit(struct cobalt *co, uint32_t id)
{
	struct dstring data = DSTR_EMPTY;
	struct cobalt_query *q;
	const char *cur_data;
	size_t cur_len;
	int rc;

	rc = co_get_task(co, id, &q);
	if (rc != CO_ENOERR) {
		eprint("cannot read task %08x: %s\n", id, co_strerror(co));
		return EXIT_FAILURE;
	}
	co_query_getdata(q, &cur_data, &cur_len);

	rc = editstr_create(&data, cur_data, cur_len);
	q = co_query_getnext(co, q);
	assert(q == NULL);
	if (rc == ECANCELED) {
		fprintf(stderr, "editor string empty, not modifying task\n");
		return EXIT_SUCCESS;
	} else if (rc != 0) {
		eprint("cannot create data from editory: %s\n", strerror(rc));
		return EXIT_FAILURE;
	}

	rc = co_mod_data(co, id, dstr(&data), dstrlen(&data));
	if (rc != CO_ENOERR) {
		eprint("failed to modify task %08x data: %s\n", id,
				co_strerror(co));
		rc = EXIT_FAILURE;
	} else {
		rc = EXIT_SUCCESS;
	}

	dstrclr(&data);

	return rc;
}

static int modify_main(void)
{
	struct cobalt *co;
	int rc;
	int err;

	if (!id.valid) {
		eprint("id required to modify task\n");
		return EXIT_FAILURE;
	}

	co = co_open(".", &err);
	if (co == NULL) {
		eprint("cannot open database: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

	if (delete.valid && delete.boolean.value) {
		rc = modify_do_delete(co, id.id.value);
	} else if (edit.valid) {
		rc = modify_do_edit(co, id.id.value);
	} else if (destination.valid) {
		rc = modify_do_move(co, id.id.value, destination.board.value);
	} else {
		eprint("invalid commandline\n");
		rc = EXIT_FAILURE;
	}

	co_free(co);

	return rc;
}

static struct module modify_module = {
	.name = "modify",
	.desc = "modify an existing task",
	.main = modify_main,
	.args = { &delete, &edit, &nogc, &id, &destination, NULL }
};

MODULE_INIT(modify_module)

