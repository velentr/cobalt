/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "argparse.h"
#include "modules.h"
#include "util.h"

static void show_usage(void)
{
	uprint("cobalt show [--format|-f <format>] [--long|-l] [--no-board|-n]"
			" [@<board>] [<id>]\n");
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
	fprintf(stderr, "\t<id>\t\tnumeric id of the board to show\n");
}

static void show_printf(const char *fmt, struct cobalt_query *task)
{
	enum { NORMAL, FORMAT, ESCAPE } state;
	const char *data;
	const char *end;
	const char *board = NULL;
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
			case 'b':
				if (board == NULL)
					board = co_query_getboard(task);
				printf("@%s", board);
				break;
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
			case '%':
			default:
				putchar(*fmt);
				break;
			}
			state = NORMAL;
			break;
		}
	}
}

static struct arg lng;
static struct arg format = {
	.name = "format",
	.desc = "string used to specify output formatting",
	.type = ARG_STRING,
	.string = {
		.lmatch = "format",
		.smatch = 'f',
	},
	.exclude = {
		&lng,
		NULL
	}
};
static struct arg lng = {
	.name = "long",
	.desc = "print the full task data",
	.type = ARG_BOOL,
	.boolean = {
		.lmatch = "long",
		.smatch = 'l',
		.value = 0,
	},
	.exclude = {
		&format,
		NULL
	}
};

static struct arg board;
static struct arg noboard;
static struct arg id = {
	.name = "id",
	.desc = "numeric id of the board to show",
	.type = ARG_ID,
	.exclude = {
		&board,
		&noboard,
		NULL
	}
};
static struct arg board = {
	.name = "board",
	.desc = "board to show all tasks from",
	.type = ARG_BOARD,
	.exclude = {
		&id,
		NULL
	}
};
static struct arg noboard = {
	.name = "no-board",
	.desc = "do not print the board name",
	.type = ARG_BOOL,
	.boolean = {
		.lmatch = "no-board",
		.smatch = 'n',
		.value = 0,
	},
	.exclude = {
		&id,
		NULL
	}
};

static int show_main(void)
{
	struct cobalt *co;
	struct cobalt_query *q;
	const char *fmt;
	int rc;
	int err;

	co = co_open(".", &err);
	if (co == NULL) {
		eprint("cannot open database: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

	if (!board.valid && !id.valid && !format.valid && !lng.boolean.value) {
		fmt = "%b\t %i  %s\n";
		rc = co_get_all(co, &q);
	} else if (!board.valid && !id.valid && !format.valid) {
		fmt = "%b\t %i\n%l";
		rc = co_get_all(co, &q);
	} else if (!board.valid && !id.valid) {
		fmt = format.string.value;
		rc = co_get_all(co, &q);
	} else if (!id.valid && !format.valid && !lng.boolean.value) {
		fmt = "  %i  %s\n";
		rc = co_get_board(co, board.board.value, &q);
	} else if (!id.valid && !format.valid) {
		fmt = "%i\n%l\n";
		rc = co_get_board(co, board.board.value, &q);
	} else if (!id.valid) {
		fmt = format.string.value;
		rc = co_get_board(co, board.board.value, &q);
	} else if (!format.valid && !lng.boolean.value) {
		fmt = "%i  %s\n";
		rc = co_get_task(co, id.id.value, &q);
	} else if (!format.valid) {
		fmt = "%i\n%l\n";
		rc = co_get_task(co, id.id.value, &q);
	} else {
		fmt = format.board.value;
		rc = co_get_task(co, id.id.value, &q);
	}

	if (rc == CO_ENOERR) {
		rc = EXIT_SUCCESS;

		if (board.valid && !noboard.boolean.value)
			printf("@%s\n", board.board.value);

		for (; q != NULL; q = co_query_getnext(co, q))
			show_printf(fmt, q);
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
	.args = {
		&format,
		&lng,
		&board,
		&id,
		&noboard,
		NULL
	}
};

MODULE_INIT(show_module)

