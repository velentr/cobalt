/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "modules.h"
#include "modify.h"
#include "util.h"

static void modify_usage(void)
{
	uprint("cobalt modify <id> @<board>\n");
}

static void modify_usage_long(void)
{
	modify_usage();
	fprintf(stderr, "\n");
	fprintf(stderr, "arguments:\n");
	fprintf(stderr, "\t<id>\tnumeric id of the task to modify\n");
	fprintf(stderr, "\t<board>\tboard to which the task is moved\n");
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

	if (cmd.board == NULL) {
		eprint("board required when moving task\n");
		return EXIT_FAILURE;
	}

	if (cmd.id == 0) {
		eprint("id required when moving task\n");
		return EXIT_FAILURE;
	}

	co = co_open(".", &err);
	if (co == NULL) {
		eprint("cannot open database: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

	rc = co_mod_attr(co, cmd.id, "board", cmd.board);

	if (rc != CO_ENOERR) {
		eprint("cannot move to board '%s': %s\n", cmd.board,
				co_strerror(co));
		rc = EXIT_FAILURE;
	} else {
		rc = EXIT_SUCCESS;
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

