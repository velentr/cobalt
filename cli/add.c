/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include <cobalt/cobalt.h>

#include "dstring.h"
#include "editstr.h"
#include "modules.h"
#include "util.h"

static void add_usage(void)
{
	uprint("cobalt add [--message|-m <message>] @<board>\n");
}

static void add_usage_long(void)
{
	add_usage();
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t--message|-m <message>\n");
	fprintf(stderr, "\t\tuse <message> as the data for the new task\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "arguments:\n");
	fprintf(stderr, "\t<board>\tboard to which the task is added\n");
}

struct arg message = {
	.name = "message",
	.desc = "message to use as the data for the new task",
	.type = ARG_STRING,
	.string = {
		.lmatch = "message",
		.smatch = 'm',
	},
	.exclude = { NULL }
};

struct arg board = {
	.name = "board",
	.desc = "board to which the task is added",
	.type = ARG_BOARD,
	.exclude = { NULL }
};

static int add_main(void)
{
	struct dstring data = DSTR_EMPTY;
	struct cobalt *co;
	const char *msg;
	size_t len;
	uint32_t id;
	int err;
	int rc;

	if (!board.valid) {
		eprint("board required when adding task\n");
		return EXIT_FAILURE;
	}

	co = co_open(".", &err);
	if (co == NULL) {
		eprint("cannot open database: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

	if (!message.valid) {
		rc = editstr_create(&data, NULL, 0);
		if (rc == ECANCELED) {
			fprintf(stderr, "editor string empty, not adding\n");
			co_free(co);
			return EXIT_FAILURE;
		} else if (rc != 0) {
			eprint("cannot create data from editor: %s\n",
					strerror(rc));
			co_free(co);
			return EXIT_FAILURE;
		}
		msg = dstr(&data);
		len = dstrlen(&data);
	} else {
		msg = message.string.value;
		len = strlen(message.string.value);
	}

	id = co_add(co, msg, len, board.board.value);
	if (id == 0) {
		eprint("cannot add object: %s\n", co_strerror(co));
		rc = EXIT_FAILURE;
	} else {
		rc = EXIT_SUCCESS;
	}

	co_free(co);
	dstrclr(&data);

	return rc;
}

static struct module add_module = {
	.name = "add",
	.desc = "add a task to the database",
	.main = add_main,
	.usage = add_usage,
	.usage_long = add_usage_long,
	.args = { &message, &board, NULL }
};

MODULE_INIT(add_module)

