/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "argparse.h"
#include "config.h"
#include "dstring.h"
#include "modules.h"
#include "util.h"

#define CONF_DOTFILE	".cobaltconfig"

static int parsehome(void)
{
	struct dstring conf;
	const char *homedir;
	int rc;

	homedir = getenv("HOME");
	if (homedir == NULL)
		return 0;

	rc = dstrcpy(&conf, homedir);
	if (rc != 0)
		return rc;

	rc = dstrcat(&conf, "/" CONF_DOTFILE);
	if (rc != 0) {
		dstrclr(&conf);
		return rc;
	}

	rc = conf_parse(dstr(&conf));
	dstrclr(&conf);

	return rc;
}

static int parsecwd(void)
{
	return conf_parse(CONF_DOTFILE);
}

static int parseall(void)
{
	int rc;

	rc = conf_init();
	if (rc != 0)
		return rc;

	parsehome();
	parsecwd();

	return 0;
}

int main(int argc, const char *argv[])
{
	struct module *mod;
	int rc = EXIT_FAILURE;

	if (argc > 1) {
		/* for now, exlicit ignore any errors; the application should
		 * continue to work even if the config cannot be parsed
		 */
		parseall();

		mod = module_get(argv[1]);
		if (mod != NULL) {
			rc = arg_parse_all(argv + 2, mod->args);
			if (rc != 0) {
				module_usage(mod);
				rc = EXIT_FAILURE;
			} else {
				rc = mod->main();
			}
		}
		conf_free();
	}
	return rc;
}

