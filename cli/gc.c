/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "argparse.h"
#include "modules.h"
#include "util.h"

static struct arg strict = {
	.name = "strict",
	.desc = "return an error if any part of the garbage collection fails",
	.type = ARG_BOOL,
	.boolean = {
		.lmatch = "strict",
		.smatch = 's',
	},
	.exclude = { NULL }
};

static int gc_main(void)
{
	struct cobalt *co;
	int rc;

	co = co_open(".", &rc);
	if (co == NULL) {
		eprint("cannot open database: %s\n", strerror(rc));
		return EXIT_FAILURE;
	}

	if (strict.valid && strict.boolean.value)
		rc = co_gc(co, CO_GC_STRICT);
	else
		rc = co_gc(co, 0);

	if (rc != CO_ENOERR && strict.valid && strict.boolean.value) {
		eprint("garbage collection failed: %s\n", co_strerror(co));
		rc = EXIT_FAILURE;
	} else {
		rc = EXIT_SUCCESS;
	}

	co_free(co);
	return EXIT_SUCCESS;
}

static struct module gc_module = {
	.name = "gc",
	.desc = "run garbage collection on the database",
	.main = gc_main,
	.args = { &strict, NULL }
};

MODULE_INIT(gc_module)

