/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef VERSION_H_
#define VERSION_H_


struct version_cmd {
	int library;
};

int version_parse(int argc, const char *argv[], struct version_cmd *cmd);


#endif /* end of include guard: VERSION_H_ */

