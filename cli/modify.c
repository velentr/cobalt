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

#include "editstr.h"
#include "modules.h"
#include "modify.h"
#include "util.h"

static void modify_usage(void)
{
	uprint("cobalt modify <id> [--delete|-d] [--edit|-e] [--no-gc|-n] "
			"[@<board>]\n");
}

static void modify_usage_long(void)
{
	modify_usage();
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t--delete|-d\n");
	fprintf(stderr, "\t\tdelete the specified task\n");
	fprintf(stderr, "\t--edit|-e\n");
	fprintf(stderr, "\t\tedit the task's data using $EDITOR\n");
	fprintf(stderr, "\t--no-gc|-n\n");
	fprintf(stderr, "\t\tdo not run garbage collection after deletion\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "arguments:\n");
	fprintf(stderr, "\t<id>\tnumeric id of the task to modify\n");
	fprintf(stderr, "\t<board>\tboard to which the task is moved\n");
}

static int modify_do_delete(struct cobalt *co, uint32_t id, int gc)
{
	int rc;

	assert(id != 0);

	rc = co_delete(co, id, gc ? CO_DEL_NGC : 0);
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

static int modify_main(int argc, const char *argv[])
{
	struct modify_cmd cmd;
	struct cobalt *co;
	int rc;
	int err;

	rc = modify_parse(argc, argv, &cmd);
	if (rc != 0) {
		modify_usage();
		return EXIT_FAILURE;
	}

	if (cmd.id == 0) {
		eprint("id required to modify task\n");
		return EXIT_FAILURE;
	}

	co = co_open(".", &err);
	if (co == NULL) {
		eprint("cannot open database: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

	if (cmd.del) {
		rc = modify_do_delete(co, cmd.id, cmd.nogc);
	} else if (cmd.edit) {
		rc = modify_do_edit(co, cmd.id);
	} else if (cmd.board != NULL) {
		rc = modify_do_move(co, cmd.id, cmd.board);
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
	.usage = modify_usage,
	.usage_long = modify_usage_long,
};

MODULE_INIT(modify_module)

