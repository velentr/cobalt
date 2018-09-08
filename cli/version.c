/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <cobalt/cobalt.h>

#include "argparse.h"
#include "modules.h"

static void version_usage(void)
{
	uprint("cobalt version [--library|-l]\n");
}

static void version_usage_long(void)
{
	version_usage();
	fprintf(stderr, "\noptions:\n");
	fprintf(stderr, "\t--library|-l\tprint version of the linked library "
			"rather than the app\n");
}

static struct arg library = {
	.name = "library",
	.desc = "print version of the linked library rather than the app",
	.type = ARG_BOOL,
	.boolean = {
		.lmatch = "library",
		.smatch = 'l',
		.value = 0,
	},
	.exclude = { NULL }
};

static int version_main(void)
{
	int major, minor, patch;

	if (library.boolean.value) {
		co_version(&major, &minor, &patch);
	} else {
		major = CO_MAJOR;
		minor = CO_MINOR;
		patch = CO_PATCH;
	}

	printf("cobalt version %d.%d.%d\n", major, minor, patch);

	return EXIT_SUCCESS;
}

static struct module version_module = {
	.name = "version",
	.desc = "print version information about the cobalt installation",
	.main = version_main,
	.usage = version_usage,
	.usage_long = version_usage_long,
	.args = { &library, NULL }
};

MODULE_INIT(version_module)

