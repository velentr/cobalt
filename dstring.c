/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dstring.h"

void dstrempty(struct dstring *str)
{
	str->type = DSTR_STACK;
	str->used = 1;
	str->stack[0] = '\0';
}

size_t dstrlen(const struct dstring *s)
{
	return s->used - 1;
}

/* find the smallest power-of-two larger than len (len should not include the
 * nil byte
 */
static size_t dstrsize(size_t len)
{
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
	assert(dstrstatic(str));

	if (str->used + space <= DSTR_STACK_SIZE)
		return 0;
	else
		return dstrconv(str, str->stack, str->used, str->used + space);
}

static int dstrgrowh(struct dstring *str, size_t space)
{
	char *buf;
	size_t newlen;

	assert(dstrdynamic(str));

	if (str->used + space <= str->heap.allocd) {
		return 0;
	} else {
		newlen = dstrsize(str->used + space - 1);
		assert(newlen >= str->used + space);

		buf = realloc(str->heap.p, newlen);
		if (buf == NULL)
			return ENOMEM;

		str->heap.p = buf;
		str->heap.allocd = newlen;

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

	return 0;
}

int dstrcpyl(struct dstring *dst, const char *src, size_t len)
{
	dst->used = len + 1;

	if (len + 1 < DSTR_STACK_SIZE)
		return dstrcpysl(dst, src, len);
	else
		return dstrcpyhl(dst, src, len);
}

static int dstrcathl(struct dstring *dst, const char *src, size_t len)
{
	int rc;

	if (dst->heap.allocd < dst->used + len) {
		rc = dstrgrowh(dst, len);
		if (rc != 0)
			return rc;
	}

	memcpy(dst->heap.p + dst->used - 1, src, len);
	dst->heap.p[dst->used + len - 1] = '\0';
	dst->used += len;

	return 0;
}

static int dstrcatsl(struct dstring *dst, const char *src, size_t len)
{
	int rc;

	if (dst->used + len <= DSTR_STACK_SIZE) {
		memcpy(dst->stack + dst->used - 1, src, len);
		dst->stack[dst->used + len - 1] = '\0';
		dst->used += len;

		return 0;
	} else {
		rc = dstrconv(dst, dst->stack, dst->used - 1, dst->used + len);
		if (rc != 0)
			return rc;

		memcpy(dst->heap.p + dst->used - 1, src, len);
		dst->heap.p[dst->used + len - 1] = '\0';
		dst->used += len;

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
	if (dstrdynamic(dst))
		free(dst->heap.p);

	dst->used = 0;
	dst->stack[0] = '\0';
	dst->type = DSTR_STACK;
}

void dstrdel(struct dstring *str, size_t len)
{
	assert(str->used > len);

	str->used -= len;
	if (dstrstatic(str))
		str->stack[str->used - 1] = '\0';
	else
		str->heap.p[str->used - 1] = '\0';
}

