/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef COBALT_H_
#define COBALT_H_


#include <stdint.h>
#include <stdlib.h>

#include <cobalt/cobalt_error.h>

/* major, minor, and patch version numbers */
#define CO_MAJOR	0
#define CO_MINOR	0
#define CO_PATCH	1

#define CO_API		__attribute__ ((visibility ("default")))

/* forward-declaration of cobalt database */
struct cobalt;
struct cobalt_query;

/* flags for co_gc() */
#define CO_GC_STRICT	(1 << 0)

/* flags for co_delete() */
#define CO_DEL_NGC	(1 << 0)
#define CO_DEL_STRICTGC	(1 << 1)

/* get version of compiled library */
void co_version(int *major, int *minor, int *patch) CO_API;
/* get error string from the last cobalt operation */
const char *co_strerror(struct cobalt *co) CO_API;

/* initialize a cobalt task list in the given directory */
struct cobalt *co_init(const char *path, int *err) CO_API;
/* open a connection to a cobalt database */
struct cobalt *co_open(const char *path, int *err) CO_API;
/* free all memory used by the given cobalt database */
void co_free(struct cobalt *co) CO_API;

/* add a new task to the cobalt database */
uint32_t co_add(struct cobalt *co, const char *data, size_t len,
		const char *board) CO_API;
/* modify an existing attribute */
int co_mod_attr(struct cobalt *co, uint32_t id, const char *name,
		const char *newval) CO_API;
/* delete an existing cobalt entry */
int co_delete(struct cobalt *co, uint32_t id, uint32_t flags) CO_API;
/* run garbage collection on the cobalt task database */
int co_gc(struct cobalt *co, uint32_t flags) CO_API;

/* get all tasks from the given board */
int co_get_board(struct cobalt *co, const char *board,
		struct cobalt_query **q) CO_API;

/* reading the results of a query */
struct cobalt_query *co_query_getnext(struct cobalt *co,
		struct cobalt_query *q) CO_API;
uint32_t co_query_getid(struct cobalt_query *q) CO_API;
const char *co_query_getboard(struct cobalt_query *q) CO_API;
void co_query_getdata(struct cobalt_query *q, const char **data,
		size_t *len) CO_API;


#endif /* end of include guard: COBALT_H_ */

