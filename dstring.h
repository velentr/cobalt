/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

/* TODO implement dstrcpy() & co and decide at compile-time which version to use
 * based on whether or not src is a string literal using __builtin_constant_p()
 */

#ifndef DSTRING_H_
#define DSTRING_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DSTR_STACK	0
#define DSTR_HEAP	1

#define DSTR_STACK_SIZE	32

/* dynamic string structure */
struct dstring {
	unsigned type:1;
	unsigned used:16;
	union {
		char stack[DSTR_STACK_SIZE];
		struct {
			char *p;
			size_t allocd;
		} heap;
	};
};

/* create an empty dstring */
void dstrempty(struct dstring *str);

/* copy the value of a dynamic string */
#define dstrcpy(dst, src) dstrcpyl(dst, src, strlen(src))
int dstrcpyl(struct dstring *dst, const char *src, size_t len);

/* get the string length (not including the nil byte) */
size_t dstrlen(const struct dstring *str);

/* get the buffer for a given string */
const char *dstr(const struct dstring *str);

/* concatenate one string onto the end of another */
#define dstrcat(dst, src) dstrcatl(dst, src, strlen(src))
int dstrcatl(struct dstring *dst, const char *src, size_t len);

/* allocate at least space more bytes for the given dynamic string */
int dstrgrow(struct dstring *str, size_t space);

/* clear a dynamic string (freeing all used memory) */
void dstrclr(struct dstring *str);

/* delete len characters from the end of a string */
void dstrdel(struct dstring *str, size_t len);


#endif /* end of include guard: DSTRING_H_ */

