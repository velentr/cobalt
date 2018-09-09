/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdlib.h>

#include "argparse.h"
#include "list.h"
#include "modules.h"

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

static struct arg shortusage = {
	.name = "short",
	.desc = "print shortened usage message",
	.type = ARG_BOOL,
	.boolean = {
		.lmatch = "short",
		.smatch = 's',
	},
	.exclude = { NULL }
};

static struct arg command = {
	.name = "command",
	.desc = "print help information for <command>",
	.type = ARG_BARE,
	.exclude = { NULL }
};

static int help_main(void)
{
	struct module *m;

	if (!command.valid) {
		help_all();
		return EXIT_SUCCESS;
	}

	m = module_get(command.bare.value);
	if (m == NULL) {
		eprint("unknown command: %s\n", command.bare.value);
		return EXIT_FAILURE;
	}

	if (shortusage.valid && shortusage.boolean.value)
		module_usage(m);
	else
		module_usage_long(m);

	return EXIT_SUCCESS;
}

static struct module help_module = {
	.name = "help",
	.desc = "print usage information",
	.main = help_main,
	.args = { &shortusage, &command, NULL }
};

MODULE_INIT(help_module)

