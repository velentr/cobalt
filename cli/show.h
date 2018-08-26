/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef SHOW_H_
#define SHOW_H_


struct show_cmd {
	const char *board;
	int noboard;
	int longf;
};

int show_parse(int argc, const char *argv[], struct show_cmd *cmd);


#endif /* end of include guard: SHOW_H_ */

