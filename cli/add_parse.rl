/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <string.h>

#include "add.h"
#include "util.h"

%%{
	machine add_parse;

	write data noerror nofinal noentry;

	action setboard {
		if (cmd->board == NULL) {
			cmd->board = arg + 1;
		} else {
			eprint("multiple boards provided\n");
			return -1;
		}
	}

	board = '@' [^\0/]+;
	path = ("--path\0" | "-p\0") [^\0]+;

	main := (( board %setboard | path ) '\0' )+;
}%%

int add_parse(int argc, const char *argv[], struct add_cmd *cmd)
{
	const char *arg, *p, *pe;
	int i;
	int cs;

	cmd->board = NULL;
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

