/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "init.h"
#include "modules.h"

static void init_usage(void)
{
	uprint("cobalt init [<path>]\n");
}

static void init_usage_long(void)
{
	init_usage();

	fprintf(stderr, "\n");
	fprintf(stderr, "arguments:\n");
	fprintf(stderr, "\t<path>\tpath to cobalt database to initialize\n");
}

static int init_main(int argc, const char *argv[])
{
	struct cobalt *co;
	struct init_cmd init;
	int rc;

	rc = init_parse(argc, argv, &init);
	if (rc != 0) {
		init_usage();
		return EXIT_FAILURE;
	}

	if (init.path == NULL)
		init.path = ".";

	co = co_init(init.path, &rc);
	if (co == NULL) {
		eprint("cannot create cobalt database: %s\n", strerror(rc));
		return EXIT_FAILURE;
	}

	co_free(co);
	return EXIT_SUCCESS;
}

static struct module init_module = {
	.name = "init",
	.desc = "initialize a cobalt task database",
	.main = init_main,
	.usage = init_usage,
	.usage_long = init_usage_long,
};

MODULE_INIT(init_module)

