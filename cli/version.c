/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <cobalt/cobalt.h>

#include "modules.h"
#include "version.h"

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

static int version_main(int argc, const char *argv[])
{
	struct version_cmd cmd;
	int major, minor, patch;
	int rc;

	rc = version_parse(argc, argv, &cmd);
	if (rc != 0) {
		version_usage();
		return EXIT_FAILURE;
	}

	if (cmd.library) {
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
};

MODULE_INIT(version_module)

