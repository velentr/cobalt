/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdlib.h>

#include "help.h"
#include "list.h"
#include "modules.h"

static void help_usage(void)
{
	uprint("cobalt help [--short|-s] [<command>]\n");
}

static void help_usage_long(void)
{
	help_usage();
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t--short|-s\tprint short usage string\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "arguments:\n");
	fprintf(stderr, "\t<command>\tprint help information for <command>\n");
}

static void help_all(void)
{
	struct list_elem *le;
	struct module *m;

	uprint("cobalt <command> [<options> ...]\n");

	fprintf(stderr, "\ncommands:\n");
	list_foreach(le, &commands) {
		m = moduleof(le);
		fprintf(stderr, "\t%s\t%s\n", m->name, m->desc);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "'cobalt help <command>' shows additional help "
			"information for <command>\n");
}

static int help_main(int argc, const char *argv[])
{
	struct help_cmd help;
	struct module *m;
	int rc;

	rc = help_parse(argc, argv, &help);
	if (rc != 0) {
		help_usage();
		return EXIT_FAILURE;
	}

	if (help.command == NULL) {
		help_all();
		return EXIT_SUCCESS;
	}

	m = module_get(help.command);
	if (m == NULL) {
		eprint("unknown command: %s\n", help.command);
		return EXIT_FAILURE;
	}

	if (help.shortusage)
		m->usage();
	else
		m->usage_long();

	return EXIT_SUCCESS;
}

static struct module help_module = {
	.name = "help",
	.desc = "print usage information",
	.main = help_main,
	.usage = help_usage,
	.usage_long = help_usage_long,
};

MODULE_INIT(help_module)

