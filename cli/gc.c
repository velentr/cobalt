/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "gc.h"
#include "modules.h"
#include "util.h"

static void gc_usage(void)
{
	uprint("cobalt gc [--strict]\n");
}

static void gc_usage_long(void)
{
	gc_usage();
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t--strict\n\t\treturn an error if any part of the "
			"garbage collection fails\n");
}

static int gc_main(int argc, const char *argv[])
{
	struct gc_cmd cmd;
	struct cobalt *co;
	int rc;

	rc = gc_parse(argc, argv, &cmd);
	if (rc != 0)
		return EXIT_FAILURE;

	co = co_open(".", &rc);
	if (co == NULL) {
		eprint("cannot open database: %s\n", strerror(rc));
		return EXIT_FAILURE;
	}

	rc = co_gc(co, cmd.strict ? CO_GC_STRICT : 0);
	if (rc != CO_ENOERR && cmd.strict) {
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
	.usage = gc_usage,
	.usage_long = gc_usage_long,
};

MODULE_INIT(gc_module)

