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
#include "show.h"
#include "util.h"

static void show_usage(void)
{
	uprint("cobalt show [--long|-l] [--no-board|-n] @<board>\n");
}

static void show_usage_long(void)
{
	show_usage();
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t--long|-l\tprint the full task data\n");
	fprintf(stderr, "\t--no-board|-n\tdo not print the board name\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "arguments:\n");
	fprintf(stderr, "\t<board>\t\tboard to show all tasks from\n");
}

static void show_print_task(struct show_cmd *cmd, struct cobalt_query *q)
{
	const char *data;
	size_t len;
	const char *endp;
	uint32_t id;

	co_query_getdata(q, &data, &len);
	id = co_query_getid(q);

	if (cmd->longf) {
		printf("%08x\n", id);
		fwrite(data, sizeof(*data), len, stdout);
		printf("\n");
	} else {
		printf("  %08x  ", id);

		endp = memchr(data, '\n', len);
		if (endp == NULL) {
			fwrite(data, sizeof(*data), len, stdout);
			putchar('\n');
		} else {
			fwrite(data, sizeof(*data), endp - data + 1, stdout);
		}
	}
}

static int show_main(int argc, const char *argv[])
{
	struct show_cmd cmd;
	struct cobalt *co;
	struct cobalt_query *q;
	int rc;
	int err;

	rc = show_parse(argc, argv, &cmd);
	if (rc != 0) {
		show_usage();
		return EXIT_FAILURE;
	}

	if (cmd.board == NULL) {
		eprint("must supply board to show from");
		return EXIT_FAILURE;
	}

	co = co_open(".", &err);
	if (co == NULL) {
		eprint("cannot open database: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

	rc = co_get_board(co, cmd.board, &q);
	if (rc == CO_ENOERR) {
		rc = EXIT_SUCCESS;

		if (!cmd.noboard)
			printf("@%s\n", cmd.board);

		for (; q != NULL; q = co_query_getnext(co, q))
			show_print_task(&cmd, q);
	} else {
		eprint("cannot query database: %s\n", co_strerror(co));
		rc = EXIT_FAILURE;
	}

	co_free(co);
	return rc;
}

static struct module show_module = {
	.name = "show",
	.desc = "show information about tasks in the database",
	.main = show_main,
	.usage = show_usage,
	.usage_long = show_usage_long,
};

MODULE_INIT(show_module)

