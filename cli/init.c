/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "argparse.h"
#include "modules.h"

struct arg path = {
	.name = "path",
	.desc = "path to the cobalt database to initialize",
	.type = ARG_BARE,
	.exclude = { NULL }
};

static int init_main(void)
{
	struct cobalt *co;
	const char *p;
	int rc;

	if (path.valid)
		p = path.bare.value;
	else
		p = ".";

	co = co_init(p, &rc);
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
	.args = { &path, NULL }
};

MODULE_INIT(init_module)

