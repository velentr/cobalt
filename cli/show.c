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
#include "config.h"
#include "dstring.h"
#include "find.h"
#include "modules.h"
#include "util.h"

static const char *show_board_color = "";
static const char *show_id_color = "";
static const char *show_short_color = "";
static const char *show_long_color = "";
static const char *show_reset_color = "";

static const char *show_colorof(const char *color)
{
	if (strcmp(color, "black") == 0)
		return "\033[30m";
	else if (strcmp(color, "red") == 0)
		return "\033[31m";
	else if (strcmp(color, "green") == 0)
		return "\033[32m";
	else if (strcmp(color, "yellow") == 0)
		return "\033[33m";
	else if (strcmp(color, "blue") == 0)
		return "\033[34m";
	else if (strcmp(color, "magenta") == 0)
		return "\033[35m";
	else if (strcmp(color, "cyan") == 0)
		return "\033[36m";
	else if (strcmp(color, "white") == 0)
		return "\033[37m";
	else
		return "\033[0m";
}

static void show_setup_colors(void)
{
	int en;

	en = conf_getb("color", "enable", 0);
	if (en) {
		show_board_color =
			show_colorof(conf_gets("color", "board", "red"));
		show_id_color =
			show_colorof(conf_gets("color", "id", "yellow"));
		show_short_color =
			show_colorof(conf_gets("color", "short", "reset"));
		show_long_color =
			show_colorof(conf_gets("color", "long", "reset"));
		show_reset_color = show_colorof("reset");
	}
}

static void show_printboard(const char *board)
{
	printf("%s@%s%s", show_board_color, board, show_reset_color);
}

static void show_printid(uint32_t id)
{
	printf("%s%08x%s", show_id_color, id, show_reset_color);
}

static void show_printshort(const char *data, size_t len)
{
	const char *end;

	fputs(show_short_color, stdout);
	end = memchr(data, '\n', len);
	if (end == NULL)
		fwrite(data, 1, len, stdout);
	else
		fwrite(data, 1, end - data, stdout);
	fputs(show_reset_color, stdout);
}

static void show_printlong(const char *data, size_t len)
{
	fputs(show_long_color, stdout);
	fwrite(data, 1, len, stdout);
	fputs(show_reset_color, stdout);
}

static void show_printf(const char *fmt, struct cobalt_query *task)
{
	enum { NORMAL, FORMAT, ESCAPE } state;
	const char *data;
	const char *board = NULL;
	size_t len;

	co_query_getdata(task, &data, &len);
	board = co_query_getboard(task);
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
				show_printboard(board);
				break;
			case 'i':
				show_printid(co_query_getid(task));
				break;
			case 'l':
				show_printlong(data, len);
				break;
			case 's':
				show_printshort(data, len);
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

	co = find_cobalt(NULL);
	if (co == NULL)
		return EXIT_FAILURE;

	show_setup_colors();

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

		if (board.valid && !noboard.boolean.value) {
			show_printboard(board.board.value);
			putchar('\n');
		}

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
	.args = { &format, &lng, &board, &id, &noboard, NULL }
};

MODULE_INIT(show_module)

