/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "modules.h"
#include "util.h"

int main(int argc, const char *argv[])
{
	struct module *mod;

	if (argc > 1) {
		mod = module_get(argv[1]);
		if (mod != NULL)
			return mod->main(argc - 2, argv + 2);
	}
	return EXIT_FAILURE;
}

