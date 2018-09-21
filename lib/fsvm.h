/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef FSVM_H_
#define FSVM_H_


#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>

#include "dstring.h"
#include "fsvm_ops.h"
#include "list.h"
#include "util.h"

#define FSVM_NUM_REGS		16
#define FSVM_NUM_ARGS		16
#define FSVM_MAX_NAME		31
#define FSVM_MAX_LNKTARG	31
#define FSVM_GLOB_NUM_REGS	4

#define to_fsvm_glob(elem) containerof(elem, struct fsvm_glob, le)

struct fsvm_arg {
	const char *data;
	size_t len;
};

struct fsvm_glob {
	char name[FSVM_MAX_NAME + 1]; /* include +1 for the nil byte */
	struct dstring reg[FSVM_GLOB_NUM_REGS];
	struct list_elem le;
	int fd;
};

struct fsvm_op {
	uint8_t type;
	uint8_t x;
	uint8_t y;
	uint8_t z;
};

struct fsvm {
	struct fsvm_arg arg[FSVM_NUM_ARGS];
	struct dstring reg[FSVM_NUM_REGS];
	struct list globs;
	DIR *index;
	size_t ip;
};

/* initialize a new fsvm */
void fsvm_init(struct fsvm *vm);

/* free all memory used by the given fsvm */
void fsvm_clear(struct fsvm *vm);

/* retrieve all globs from the given vm */
void fsvm_globs(struct fsvm *vm, struct list *globs);

/* free a glob's memory */
void fsvm_globfree(struct fsvm_glob *g);

/* load a new argument into the fsvm */
void fsvm_ldarg(struct fsvm *vm, size_t argno, const char *argv, size_t arglen);

/* run an opcode sequence on an fsvm */
int fsvm_run(struct fsvm *vm, const struct fsvm_op *op, size_t nops);


#endif /* end of include guard: FSVM_H_ */

