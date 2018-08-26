/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef STRUCT_H_
#define STRUCT_H_


#include "db.h"
#include "dstring.h"
#include "list.h"
#include "random.h"

/* main cobalt structure, should not be exposed externally */
struct cobalt {
	struct co_db db;
	struct co_rng rng;
	struct dstring path;
	int err;
	char errstr[1024];
};

/* result of a query */
struct cobalt_query {
	struct dstring board;
	struct list_elem le;
	char *data;
	size_t len;
	uint32_t id;
};


#endif /* end of include guard: STRUCT_H_ */

