/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef GC_H_
#define GC_H_


struct gc_cmd {
	int strict;
};

int gc_parse(int argc, const char *argv[], struct gc_cmd *cmd);


#endif /* end of include guard: GC_H_ */

