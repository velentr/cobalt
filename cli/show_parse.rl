/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <string.h>

#include "show.h"
#include "util.h"

%%{
	machine show_parse;

	write data noerror nofinal noentry;

	action setboard {
		if (cmd->board == NULL) {
			cmd->board = arg + 1;
		} else {
			eprint("multiple boards provided\n");
			return -1;
		}
	}

	action setnoboard {
		cmd->noboard = 1;
	}

	action setlong {
		cmd->longf = 1;
	}

	board = '@' [^\0/]+;
	noboard = ("--no-board" | "-n");
	long = ("--long" | "-l");

	main := (( board %setboard | noboard %setnoboard | long %setlong )
			      '\0' )+;
}%%

int show_parse(int argc, const char *argv[], struct show_cmd *cmd)
{
	const char *arg, *p, *pe;
	int i;
	int cs;

	cmd->board = NULL;
	cmd->noboard = 0;
	cmd->longf = 0;

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

