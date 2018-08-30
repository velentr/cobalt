/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef MODIFY_H_
#define MODIFY_H_


#include <stdint.h>

struct modify_cmd {
	const char *board;
	uint32_t id;
	int del;
	int nogc;
};

int modify_parse(int argc, const char *argv[], struct modify_cmd *cmd);


#endif /* end of include guard: MODIFY_H_ */

