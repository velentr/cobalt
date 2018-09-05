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
		cmd->lng = 1;
	}

	action setformat {
		cmd->format = arg;
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
	noboard = ("--no-board" | "-n");
	long = ("--long" | "-l");
	format = ( "--format\0" | "-f\0" ) [^\0]+;
	id = [0-9a-fA-F]{8} $adddigit;

	main := (( board %setboard | noboard %setnoboard | long %setlong |
			format %setformat | id ) '\0' )+;
}%%

int show_parse(int argc, const char *argv[], struct show_cmd *cmd)
{
	const char *arg, *p, *pe;
	int i;
	int cs;

	cmd->board = NULL;
	cmd->format = NULL;
	cmd->lng = 0;
	cmd->noboard = 0;
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

