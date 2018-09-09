/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "modules.h"

LIST_INIT(commands);

struct module *module_get(const char *name)
{
	struct list_elem *le;
	struct module *m;

	list_foreach(le, &commands) {
		m = moduleof(le);
		if (strcmp(m->name, name) == 0)
			return m;
	}

	return NULL;
}

/* get the number of columns to use for printing */
static int module_getcols(void)
{
	char *val;

	val = getenv("COLUMNS");
	/* use 80 columns by default */
	if (val == NULL)
		return 80;

	return atoi(val);
}

/* get the number of characters that would be printed for the given arg */
int module_printlen(struct arg *a)
{
	switch (a->type) {
	case ARG_BOARD:
		return strlen(" @<>") + strlen(a->name);
	case ARG_ID:
		return strlen(" <>") + strlen(a->name);
	case ARG_STRING:
		return strlen(" [-- <>]") + strlen(a->string.lmatch) +
			strlen(a->name);
	case ARG_BOOL:
		return strlen(" [--]") + strlen(a->boolean.lmatch);
	case ARG_BARE:
		return strlen(" <>") + strlen(a->name);
	default:
		assert(0);
		return 0;
	}
}

/* print the given argument */
int module_printarg(struct arg *a)
{
	switch (a->type) {
	case ARG_BOARD:
		return fprintf(stderr, " @<%s>", a->name);
	case ARG_ID:
		return fprintf(stderr, " <%s>", a->name);
	case ARG_STRING:
		return fprintf(stderr, " [--%s <%s>]", a->string.lmatch,
				a->name);
	case ARG_BOOL:
		return fprintf(stderr, " [--%s]", a->boolean.lmatch);
	case ARG_BARE:
		return fprintf(stderr, " <%s>", a->name);
	default:
		assert(0);
		return 0;
	}
}

void module_usage(struct module *mod)
{
	int cols;
	int remaining;
	size_t i;

	cols = module_getcols();
	remaining = cols - fprintf(stderr, "usage: cobalt %s", mod->name);

	for (i = 0; mod->args[i] != NULL; i++) {
		if (remaining - module_printlen(mod->args[i]) >= 0) {
			remaining -= module_printarg(mod->args[i]);
		} else {
			fprintf(stderr, "\n       ");
			remaining = cols - 7 - module_printarg(mod->args[i]);
		}
	}
	fprintf(stderr, "\n");
}

static void module_printargname(struct arg *a)
{
	switch (a->type) {
	case ARG_BOARD:
		fprintf(stderr, "    @<%s>", a->name);
		break;
	case ARG_ID:
		fprintf(stderr, "    <%s>", a->name);
		break;
	case ARG_STRING:
		fprintf(stderr, "    -%c <%s>, --%s <%s>", a->string.smatch,
				a->name, a->string.lmatch, a->name);
		break;
	case ARG_BOOL:
		fprintf(stderr, "    -%c, --%s", a->boolean.smatch,
				a->boolean.lmatch);
		break;
	case ARG_BARE:
		fprintf(stderr, "    <%s>", a->name);
		break;
	default:
		assert(0);
		break;
	}
}

static void module_printargline(int cols, struct arg *a)
{
	int remaining = 0;
	const char *start = a->desc;
	const char *end = strchrnul(start, ' ');

	while (*start != '\0') {
		if (remaining - (end - start + 1) < 0) {
			fprintf(stderr, "\n        ");
			fwrite(start, sizeof(char), end - start, stderr);
			remaining = cols - 8 - (end - start);
		} else {
			remaining -= (end - start + 1);
			fprintf(stderr, " ");
			fwrite(start, sizeof(char), end - start, stderr);
		}

		if (*end == '\0')
			break;

		start = end + 1;
		end = strchrnul(start, ' ');
	}
	fprintf(stderr, "\n");
}

void module_usage_long(struct module *mod)
{
	int cols;
	size_t i;

	module_usage(mod);
	fprintf(stderr, "\n");

	cols = module_getcols();
	fprintf(stderr, "options:\n");

	for (i = 0; mod->args[i] != NULL; i++) {
		module_printargname(mod->args[i]);
		module_printargline(cols, mod->args[i]);
	}
}

