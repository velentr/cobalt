/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef ARGPARSE_H_
#define ARGPARSE_H_


#include <stdint.h>

#include "list.h"

enum arg_type_t {
	ARG_BOARD,
	ARG_ID,
	ARG_BOOL,
	ARG_STRING,
	ARG_BARE,
};

/* a board argument matches against @value on the command-line */
struct arg_board {
	const char *value;
};

/* an id argument matches [0-9a-fA-F]{8} on the command-line, and will be parsed
 * into a 32-bit unsigned integer
 */
struct arg_id {
	uint32_t value;
};

/* a boolean arg matches --lmatch|-smatch, and is indicated as a boolean */
struct arg_bool {
	const char *lmatch;
	int value;
	char smatch;
};

/* a string arg matches --lmatch value|-smatch value, and the resulting string
 * is stored as the value
 */
struct arg_string {
	const char *lmatch;
	const char *value;
	char smatch;
};

/* bare arguments match anything, so make sure they are always last in the
 * module's args array
 */
struct arg_bare {
	const char *value;
};

struct arg {
	/* these fields should be initialized statically */
	const char *name;
	const char *desc;
	enum arg_type_t type;
	union {
		struct arg_board board;
		struct arg_id id;
		struct arg_bool boolean;
		struct arg_string string;
		struct arg_bare bare;
	};

	/* everything else should be left alone and will be used internally by
	 * the argparse module
	 */
	int valid;
	int active;
	struct list_elem le;

	/* this array should also be set statically; it must be last because its
	 * size may vary per argument
	 */
	struct arg *exclude[];
};

int arg_parse_all(const char *argv[], struct arg **args);


#endif /* end of include guard: ARGPARSE_H_ */

