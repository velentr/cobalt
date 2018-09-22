/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include <cobalt/cobalt.h>

#include "dstring.h"
#include "editstr.h"
#include "find.h"
#include "modules.h"
#include "util.h"

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
	struct dstring data;
	struct cobalt *co;
	const char *msg;
	size_t len;
	uint32_t id;
	int rc;

	dstrempty(&data);

	if (!board.valid) {
		eprint("board required when adding task\n");
		return EXIT_FAILURE;
	}

	co = find_cobalt(NULL);
	if (co == NULL)
		return EXIT_FAILURE;

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
	.args = { &message, &board, NULL }
};

MODULE_INIT(add_module)

