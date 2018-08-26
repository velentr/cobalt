/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef INIT_H_
#define INIT_H_


struct init_cmd {
	const char *path;
};

int init_parse(int argc, const char *argv[], struct init_cmd *cmd);


#endif /* end of include guard: INIT_H_ */

