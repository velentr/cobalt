/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <stdio.h>
#include <string.h>

#include "gc.h"
#include "util.h"

%%{
	machine gc_parse;

	write data noerror nofinal noentry;

	action setstrict {
		cmd->strict = 1;
	}

	strict = "--strict" | "-s";

	main := (( strict %setstrict ) '\0' )+;
}%%

int gc_parse(int argc, const char *argv[], struct gc_cmd *cmd)
{
	const char *arg, *p, *pe;
	int i;
	int cs;

	cmd->strict = 0;

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

