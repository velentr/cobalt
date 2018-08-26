/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef DB_H_
#define DB_H_


#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>

#include <cobalt/cobalt.h>

#include "dstring.h"
#include "list.h"
#include "opcodes.h"

#define CO_DB_NSTR	6

/* in-memory cache of database */
struct co_db {
	struct dstring str[CO_DB_NSTR];
	struct list query;
	struct list_elem *cursor;
	DIR *qdir;
	size_t ip;
};

/* initialize the database cache */
void co_db_init(struct co_db *db);

/* run a transaction on the database */
int co_db_run(struct co_db *db, const struct co_db_op *ops, size_t nops);

/* free the database cache */
void co_db_free(struct co_db *db);

/* free a query entry */
void co_db_query_free(struct cobalt_query *q);


#endif /* end of include guard: DB_H_ */

