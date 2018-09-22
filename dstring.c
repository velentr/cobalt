/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef CO_VALGRIND
#include <valgrind/memcheck.h>
#else
#define VALGRIND_MAKE_MEM_NOACCESS(addr, len)	(void)0
#define VALGRIND_MAKE_MEM_UNDEFINED(addr, len)	(void)0
#endif

#include "dstring.h"

#define DSTR_STACK_LOCK(str) \
	VALGRIND_MAKE_MEM_NOACCESS(str->stack + str->used, \
			sizeof(str->stack) - str->used)
#define DSTR_STACK_UNLOCK(str) \
	VALGRIND_MAKE_MEM_UNDEFINED(str->stack + str->used, \
			sizeof(str->stack) - str->used)
#define DSTR_STACK_NEW(str) \
	VALGRIND_MAKE_MEM_UNDEFINED(str->stack, sizeof(str->stack))

#define DSTR_HEAP_LOCK(str) \
	VALGRIND_MAKE_MEM_NOACCESS(str->heap.p + str->used, \
			str->heap.allocd - str->used)
#define DSTR_HEAP_UNLOCK(str) \
	VALGRIND_MAKE_MEM_UNDEFINED(str->heap.p + str->used, \
			str->heap.allocd - str->used)

void dstrempty(struct dstring *str)
{
	DSTR_STACK_NEW(str);

	str->type = DSTR_STACK;
	str->used = 1;
	str->stack[0] = '\0';

	DSTR_STACK_LOCK(str);
}

size_t dstrlen(const struct dstring *s)
{
	return s->used - 1;
}

/* find the smallest power-of-two larger than len (len should not include the
 * nil byte
 */
static size_t dstrsize(size_t _len)
{
	unsigned len = (unsigned)_len;
	return 1 << ((8*sizeof(len)) - __builtin_clz(len));
}

/* check if a string is static or dynamic */
static int dstrstatic(const struct dstring *str)
{
	return str->type == DSTR_STACK;
}
static int dstrdynamic(const struct dstring *str)
{
	return str->type == DSTR_HEAP;
}

static int dstrconv(struct dstring *dst, const char *src, size_t len,
		size_t total)
{
	char *buf;
	size_t alloc;

	assert(len <= total);

	alloc = dstrsize(total);
	assert(total < alloc);

	buf = malloc(alloc * sizeof(*src));
	if (buf == NULL)
		return ENOMEM;

	memcpy(buf, src, len);

	dst->heap.p = buf;
	dst->heap.allocd = alloc;
	dst->type = DSTR_HEAP;

	return 0;
}

static int dstrgrows(struct dstring *str, size_t space)
{
	int rc;

	assert(dstrstatic(str));

	if (str->used + space <= DSTR_STACK_SIZE) {
		rc = 0;
	} else {
		DSTR_STACK_UNLOCK(str);
		rc = dstrconv(str, str->stack, str->used, str->used + space);
		DSTR_HEAP_LOCK(str);
	}

	return rc;
}

static int dstrgrowh(struct dstring *str, size_t space)
{
	char *buf;
	size_t newlen;

	assert(dstrdynamic(str));

	if (str->used + space <= str->heap.allocd) {
		return 0;
	} else {
		DSTR_HEAP_UNLOCK(str);

		newlen = dstrsize(str->used + space - 1);
		assert(newlen >= str->used + space);
		buf = realloc(str->heap.p, newlen);
		if (buf == NULL)
			return ENOMEM;

		str->heap.p = buf;
		str->heap.allocd = newlen;

		DSTR_HEAP_LOCK(str);

		return 0;
	}
}

int dstrgrow(struct dstring *str, size_t space)
{
	if (dstrstatic(str))
		return dstrgrows(str, space);
	else
		return dstrgrowh(str, space);
}

const char *dstr(struct dstring *str)
{
	if (dstrstatic(str))
		return str->stack;
	else
		return str->heap.p;
}

static int dstrcpysl(struct dstring *dst, const char *src, size_t len)
{
	dst->type = DSTR_STACK;
	memcpy(dst->stack, src, len);
	dst->stack[len] = '\0';

	assert(dstrstatic(dst));
	DSTR_STACK_LOCK(dst);

	return 0;
}

static int dstrcpyhl(struct dstring *dst, const char *src, size_t len)
{
	int rc;

	rc = dstrconv(dst, src, len, len);
	if (rc != 0)
		return rc;
	assert(len < dst->heap.allocd);

	dst->heap.p[len] = '\0';

	assert(dstrdynamic(dst));
	DSTR_HEAP_LOCK(dst);

	return 0;
}

int dstrcpyl(struct dstring *dst, const char *src, size_t len)
{
	DSTR_STACK_NEW(dst);

	dst->used = len + 1;
	if (len + 1 < DSTR_STACK_SIZE)
		return dstrcpysl(dst, src, len);
	else
		return dstrcpyhl(dst, src, len);
}

static int dstrcathl(struct dstring *dst, const char *src, size_t len)
{
	int rc;

	DSTR_HEAP_UNLOCK(dst);

	if (dst->heap.allocd < dst->used + len) {
		rc = dstrgrowh(dst, len);
		if (rc != 0) {
			DSTR_HEAP_LOCK(dst);
			return rc;
		}
		DSTR_HEAP_UNLOCK(dst);
	}

	memcpy(dst->heap.p + dst->used - 1, src, len);
	dst->heap.p[dst->used + len - 1] = '\0';
	dst->used += len;

	DSTR_HEAP_LOCK(dst);

	return 0;
}

static int dstrcatsl(struct dstring *dst, const char *src, size_t len)
{
	int rc;

	DSTR_STACK_UNLOCK(dst);

	if (dst->used + len <= DSTR_STACK_SIZE) {
		memcpy(dst->stack + dst->used - 1, src, len);
		dst->stack[dst->used + len - 1] = '\0';
		dst->used += len;

		DSTR_STACK_LOCK(dst);
		return 0;
	} else {
		rc = dstrconv(dst, dst->stack, dst->used - 1, dst->used + len);
		if (rc != 0) {
			DSTR_STACK_LOCK(dst);
			return rc;
		}

		memcpy(dst->heap.p + dst->used - 1, src, len);
		dst->heap.p[dst->used + len - 1] = '\0';
		dst->used += len;

		DSTR_HEAP_LOCK(dst);
		return 0;
	}
}

int dstrcatl(struct dstring *dst, const char *src, size_t len)
{
	if (dstrstatic(dst))
		return dstrcatsl(dst, src, len);
	else
		return dstrcathl(dst, src, len);
}

void dstrclr(struct dstring *dst)
{
	if (dstrdynamic(dst)) {
		DSTR_HEAP_UNLOCK(dst);
		free(dst->heap.p);
	} else {
		DSTR_STACK_UNLOCK(dst);
	}

	dstrempty(dst);
}

void dstrdel(struct dstring *str, size_t len)
{
	assert(str->used > len);

	str->used -= len;
	if (dstrstatic(str)) {
		str->stack[str->used - 1] = '\0';
		DSTR_STACK_LOCK(str);
	} else {
		str->heap.p[str->used - 1] = '\0';
		DSTR_HEAP_LOCK(str);
	}
}

