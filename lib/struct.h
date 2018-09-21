/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef STRUCT_H_
#define STRUCT_H_


#include "dstring.h"
#include "fsvm.h"
#include "random.h"

#define query_to_glob(q)	((struct fsvm_glob *)q)
#define glob_to_query(g)	((struct cobalt_query *)g)

/* main cobalt structure, should not be exposed externally */
struct cobalt {
	struct co_rng rng;
	struct dstring path;
	int err;
	char errstr[1024];
	struct list globs;
};


#endif /* end of include guard: STRUCT_H_ */

