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
	uprint("cobalt show [--format|-f <format>] [--long|-l] [--no-board|-n]"
			" @<board>\n");
}

static void show_usage_long(void)
{
	show_usage();
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t--format|-f <format>\n");
	fprintf(stderr, "\t\t\tprint output according to <format>\n");
	fprintf(stderr, "\t--long|-l\tprint the full task data\n");
	fprintf(stderr, "\t--no-board|-n\tdo not print the board name\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "arguments:\n");
	fprintf(stderr, "\t<board>\t\tboard to show all tasks from\n");
}
static void show_printf(const char *fmt, struct cobalt_query *task)
{
	enum { NORMAL, FORMAT, ESCAPE } state;
	const char *data;
	const char *end;
	size_t len;

	co_query_getdata(task, &data, &len);
	for (state = NORMAL; *fmt != '\0'; fmt++) {
		switch (state) {
		case NORMAL:
			switch (*fmt) {
			case '%':
				state = FORMAT;
				break;
			case '\\':
				state = ESCAPE;
				break;
			default:
				putchar(*fmt);
				break;
			}
			break;
		case FORMAT:
			switch (*fmt) {
			case 'i':
				printf("%08x", co_query_getid(task));
				break;
			case 'l':
				fwrite(data, 1, len, stdout);
				break;
			case 's':
				end = memchr(data, '\n', len);
				if (end == NULL)
					fwrite(data, 1, len, stdout);
				else
					fwrite(data, 1, end - data, stdout);
				break;
			default:
				putchar('%');
				putchar(*fmt);
				break;
			}
			state = NORMAL;
			break;
		case ESCAPE:
			switch (*fmt) {
			case '0':
				putchar('\0');
				break;
			case 'n':
				putchar('\n');
				break;
			case 'r':
				putchar('\r');
				break;
			case 't':
				putchar('\t');
				break;
			case '\\':
			default:
				putchar(*fmt);
				break;
			}
			state = NORMAL;
			break;
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
		eprint("must supply board to show from\n");
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
			show_printf(cmd.format, q);
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

