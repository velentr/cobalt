/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef ADD_H_
#define ADD_H_


struct add_cmd {
	const char *board;
	const char *path;
};

int add_parse(int argc, const char *argv[], struct add_cmd *cmd);


#endif /* end of include guard: ADD_H_ */

