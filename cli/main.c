/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "argparse.h"
#include "modules.h"
#include "util.h"

int main(int argc, const char *argv[])
{
	struct module *mod;
	int rc;

	if (argc > 1) {
		mod = module_get(argv[1]);
		if (mod != NULL) {
			rc = arg_parse_all(argv + 2, mod->args);
			if (rc != 0) {
				module_usage(mod);
				return EXIT_FAILURE;
			} else {
				return mod->main();
			}
		}
	}
	return EXIT_FAILURE;
}

