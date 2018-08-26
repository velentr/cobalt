/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdlib.h>

#include "modify.h"
#include "util.h"

%%{
	machine modify_parse;

	write data noerror nofinal noentry;

	action setboard {
		if (cmd->board == NULL) {
			cmd->board = arg + 1;
		} else {
			eprint("multiple boards provided\n");
			return -1;
		}
	}

	action adddigit {
		cmd->id <<= 4;
		if ('0' <= fc && fc <= '9')
			cmd->id |= fc - '0';
		else if ('a' <= fc && fc <= 'f')
			cmd->id |= fc - 'a' + 10;
		else
			cmd->id |= fc - 'A' + 10;
	}

	board = '@' [^\0/]+;
	id = [0-9a-fA-F]{8} $adddigit;

	main := (( board %setboard | id ) '\0' )+;
}%%

int modify_parse(int argc, const char *argv[], struct modify_cmd *cmd)
{
	const char *arg, *p, *pe;
	int i;
	int cs;

	cmd->board = NULL;
	cmd->id = 0;

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


