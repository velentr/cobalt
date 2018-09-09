/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef MODULES_H_
#define MODULES_H_


#include <stdio.h>

#include "argparse.h"
#include "list.h"
#include "util.h"

/* module for interacting with the cobalt database */
struct module {
	struct list_elem le;
	const char *name;
	const char *desc;
	int (*const main)(void);
	struct arg *args[];
};

/* list of commands, initialized at startup using the MODULE_INIT() macro */
extern struct list commands;

/* get the module corresponding to the given name */
struct module *module_get(const char *name);

/* print the usage string for the given module */
void module_usage(struct module *mod);
/* print the long-form usage message for the given module */
void module_usage_long(struct module *mod);

/* add a module to the command list on startup */
#define MODULE_INIT(mod) \
static void __attribute__ ((constructor)) _ ## name ## _init(void) { \
	list_pushback(&commands, &((&(mod))->le)); \
}

/* get the module from the given list element */
#define moduleof(le) containerof(le, struct module, le)


#endif /* end of include guard: MODULES_H_ */

