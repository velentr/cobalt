/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef COBALT_H_
#define COBALT_H_


#include <stdint.h>

#include <cobalt/cobalt_error.h>

/* major, minor, and patch version numbers */
#define CO_MAJOR	0
#define CO_MINOR	0
#define CO_PATCH	0

#define CO_API		__attribute__ ((visibility ("default")))

/* forward-declaration of cobalt database */
struct cobalt;
struct cobalt_query;

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

