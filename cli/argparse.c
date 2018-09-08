/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "argparse.h"
#include "list.h"
#include "util.h"

static int arg_match_board(const char **argv[], struct arg *a)
{
	const char *arg = (*argv)[0];

	if (arg[0] == '@' && arg[1] != '\0') {
		a->board.value = arg + 1;
		(*argv)++;
		return 1;
	} else {
		return 0;
	}
}

static int arg_match_id(const char **argv[], struct arg *a)
{
	const char *arg = (*argv)[0];
	unsigned long val;
	char *endp;

	val = strtoul(arg, &endp, 16);
	if (endp - arg != 8 || *endp != '\0') {
		return 0;
	} else {
		a->id.value = val;
		(*argv)++;
		return 1;
	}
}

static int arg_match_bool(const char **argv[], struct arg *a)
{
	const char *arg = (*argv)[0];

	/* long-form match */
	if ((arg[0] == '-' && arg[1] == '-' &&
				strcmp(arg + 2, a->boolean.lmatch) == 0)
	/* short-form match */
	|| (arg[0] == '-' && arg[1] == a->boolean.smatch && arg[2] == '\0')) {
		a->boolean.value = 1;
		(*argv)++;
		return 1;
	} else {
		return 0;
	}
}

static int arg_match_string(const char **argv[], struct arg *a)
{
	const char *arg = (*argv)[0];
	const char *val = (*argv)[1];

	if (val == NULL)
		return 0;

	/* long-form match */
	if ((arg[0] == '-' && arg[1] == '-' &&
				strcmp(arg + 2, a->string.lmatch) == 0)
	/* short-form match */
	|| (arg[0] == '-' && arg[1] == a->string.smatch && arg[2] == '\0')) {
		a->string.value = val;
		(*argv) += 2;
		return 1;
	} else {
		return 0;
	}
}

static int arg_match_bare(const char **argv[], struct arg *a)
{
	a->bare.value = (*argv)[0];
	(*argv)++;
	return 1;
}

static void arg_process_match(struct arg *a)
{
	size_t i;

	for (i = 0; a->exclude[i] != NULL; i++) {
		if (a->exclude[i]->active) {
			a->exclude[i]->active = 0;
			list_remove(&a->exclude[i]->le);
		}
	}

	a->active = 0;
	a->valid = 1;
	list_remove(&a->le);
}

static int arg_parse_one(const char **argv[], struct list *args)
{
	struct list_elem *e;
	struct arg *a;
	int matched;

	list_foreach(e, args) {
		a = containerof(e, struct arg, le);
		switch (a->type) {
		case ARG_BOARD:
			matched = arg_match_board(argv, a);
			break;
		case ARG_ID:
			matched = arg_match_id(argv, a);
			break;
		case ARG_BOOL:
			matched = arg_match_bool(argv, a);
			break;
		case ARG_STRING:
			matched = arg_match_string(argv, a);
			break;
		case ARG_BARE:
			matched = arg_match_bare(argv, a);
			break;
		}
		if (matched) {
			arg_process_match(a);
			return 0;
		}
	}

	return -1;
}

int arg_parse_all(const char *argv[], struct arg **args)
{
	LIST_INIT(active);
	size_t i;
	int rc;

	for (i = 0; args[i] != NULL; i++) {
		list_pushback(&active, &args[i]->le);
		args[i]->valid = 0;
		args[i]->active = 1;
	}

	while (*argv != NULL) {
		rc = arg_parse_one(&argv, &active);
		if (rc == -1) {
			eprint("unrecognized argument: %s\n", *argv);
			return -1;
		}
	}

	return 0;
}

