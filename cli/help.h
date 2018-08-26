/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef HELP_H_
#define HELP_H_


struct help_cmd {
	int shortusage;
	const char *command;
};

int help_parse(int argc, const char *argv[], struct help_cmd *cmd);


#endif /* end of include guard: HELP_H_ */

