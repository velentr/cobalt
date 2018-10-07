/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef UTIL_H_
#define UTIL_H_


#include <stdio.h>

/* get a pointer to the structure containing the given element */
#define containerof(ELEM, STRUCT, MEMBER) ((STRUCT *)((size_t)(ELEM) \
			- offsetof(STRUCT, MEMBER)))

/* find the length of a static array */
#define lengthof(arr) (sizeof(arr) / sizeof(arr[0]))

/* print an error message */
#define eprint(...) fprintf(stderr, "error: " __VA_ARGS__)

/* print a usage message */
#define uprint(...) fprintf(stderr, "usage: " __VA_ARGS__)

/* macro to make this attribute a little shorter */
#define UNUSED	__attribute__((unused))

/* pointer to a function for comparing two items */
typedef int (*cmp_func)(const void *, const void *);

#endif /* end of include guard: UTIL_H_ */

