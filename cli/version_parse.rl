/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <string.h>

#include "version.h"
#include "util.h"

%%{
	machine version_parse;

	write data noerror nofinal noentry;

	action addlibrary {
		cmd->library = 1;
	}

	action setcommand {
		if (cmd->command == NULL) {
			cmd->command = arg;
		} else {
			eprint("multiple commands provided\n");
			return -1;
		}
	}

	library = "--library" | "-l";

	main := (( library %addlibrary ) '\0' )+;
}%%

int version_parse(int argc, const char *argv[], struct version_cmd *cmd)
{
	const char *arg, *p, *pe;
	int i;
	int cs;

	cmd->library = 0;

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

