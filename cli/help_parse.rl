/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <string.h>

#include "help.h"
#include "util.h"

%%{
	machine help_parse;

	write data noerror nofinal noentry;

	action addshort {
		cmd->shortusage = 1;
	}

	action setcommand {
		if (cmd->command == NULL) {
			cmd->command = arg;
		} else {
			eprint("multiple commands provided\n");
			return -1;
		}
	}

	short = "--short" | "-s";
	command = [a-zA-Z0-9] [\-a-zA-Z0-9_]*;

	main := (( short %addshort | command %setcommand ) '\0' )+;
}%%

int help_parse(int argc, const char *argv[], struct help_cmd *cmd)
{
	const char *arg, *p, *pe;
	int i;
	int cs;

	cmd->shortusage = 0;
	cmd->command = NULL;

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

