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
#define CO_MINOR	1
#define CO_PATCH	0

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
CO_API void co_version(int *major, int *minor, int *patch);

/* get error string from the last cobalt operation */
CO_API const char *co_strerror(struct cobalt *co);

/* initialize a cobalt task list in the given directory */
CO_API struct cobalt *co_init(const char *path, int *err);

/* open a connection to a cobalt database */
CO_API struct cobalt *co_open(const char *path, int *err);

/* free all memory used by the given cobalt database */
CO_API void co_free(struct cobalt *co);

/* add a new task to the cobalt database */
CO_API uint32_t co_add(struct cobalt *co, const char *data, size_t len,
		const char *board);

/* modify an existing attribute */
CO_API int co_mod_attr(struct cobalt *co, uint32_t id, const char *name,
		const char *newval);

/* set the data for the given command */
CO_API int co_mod_data(struct cobalt *co, uint32_t id, const char *data,
		size_t len);

/* delete an existing cobalt entry */
CO_API int co_delete(struct cobalt *co, uint32_t id, uint32_t flags);

/* run garbage collection on the cobalt task database */
CO_API int co_gc(struct cobalt *co, uint32_t flags);

/* get the task with the given id */
CO_API int co_get_task(struct cobalt *co, uint32_t id, struct cobalt_query **q);

/* get all tasks from the given board */
CO_API int co_get_board(struct cobalt *co, const char *board,
		struct cobalt_query **q);

/* reading the results of a query */
CO_API struct cobalt_query *co_query_getnext(struct cobalt *co,
		struct cobalt_query *q);
CO_API uint32_t co_query_getid(struct cobalt_query *q);
CO_API const char *co_query_getboard(struct cobalt_query *q);
CO_API void co_query_getdata(struct cobalt_query *q, const char **data,
		size_t *len);


#endif /* end of include guard: COBALT_H_ */

