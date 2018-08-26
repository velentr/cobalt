/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <string.h>

#include "init.h"
#include "util.h"

%%{
	machine init_parse;

	write data noerror nofinal noentry;

	action setpath {
		if (cmd->path == NULL) {
			cmd->path = arg;
		} else {
			eprint("cannot init multiple paths\n");
			return -1;
		}
	}

	path = [^\0]+;

	main := ( path %setpath '\0' )+;
}%%

int init_parse(int argc, const char *argv[], struct init_cmd *cmd)
{
	const char *arg, *p, *pe;
	int i;
	int cs;

	cmd->path = NULL;

	%% write init;

	for (i = 0; i < argc; i++) {
		arg = argv[i];
		p = arg;
		pe = p + strlen(arg) + 1;

		%% write exec;

		if (cs == %%{ write error; }%%) {
			eprint("unknown argument: %s\n", arg);
			return -1;
		}
	}

	return 0;
}

