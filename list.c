/*
 * Copyright (C) 2017-18 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <stdlib.h>

#include "list.h"

void list_init(struct list *l)
{
	assert(l != NULL);

	l->sentinal.prev = l->sentinal.next = &l->sentinal;
}

void list_insert(struct list_elem *prev, struct list_elem *to_add)
{
	assert(prev != NULL);
	assert(to_add != NULL);

	/* Update the pointers in 'to_add'. */
	to_add->next = prev->next;
	to_add->prev = prev;

	/* Insert 'to_add' immediately following 'prev'. */
	prev->next->prev = to_add;
	prev->next = to_add;
}

struct list_elem *list_remove(struct list_elem *e)
{
	assert(e != NULL);

	/* Update the pointers in the next and previous elements. */
	e->prev->next = e->next;
	e->next->prev = e->prev;

	return e;
}

struct list_elem *list_popfront(struct list *l)
{
	struct list_elem *e;

	assert(l != NULL);
	assert(!list_isempty(l));

	/* Get the first element and remove it. */
	e = list_head(l);
	return list_remove(e);
}

struct list_elem *list_popback(struct list *l)
{
	struct list_elem *e;

	assert(l != NULL);
	assert(!list_isempty(l));

	/* Get the last element and remove it. */
	e = list_tail(l);
	return list_remove(e);
}

size_t list_size(const struct list *l)
{
	struct list_elem *e;
	size_t num = 0;

	assert(l != NULL);

	list_foreach(e, l)
		num++;

	return num;
}

void list_cat(struct list *dst, struct list *src)
{
	struct list_elem *head_src, *tail_src, *tail_dst;

	if (list_isempty(src))
		return;

	head_src = list_head(src);
	tail_src = list_tail(src);
	tail_dst = list_tail(dst);

	tail_dst->next = head_src;
	head_src->prev = tail_dst;
	tail_src->next = &dst->sentinal;
	dst->sentinal.prev = tail_src;
}

struct list_elem *list_head(const struct list *l)
{
	assert(l != NULL);

	return l->sentinal.next;
}

struct list_elem *list_tail(const struct list *l)
{
	assert(l != NULL);

	return l->sentinal.prev;
}

struct list_elem *list_begin(const struct list *l)
{
	assert(l != NULL);

	return list_head(l);
}

struct list_elem *list_next(const struct list_elem *e)
{
	assert(e != NULL);

	return e->next;
}

struct list_elem *list_prev(const struct list_elem *e)
{
	assert(e != NULL);

	return e->prev;
}

struct list_elem *list_end(const struct list *l)
{
	assert(l != NULL);

	return (struct list_elem *)&l->sentinal;
}

void list_pushfront(struct list *l, struct list_elem *e)
{
	assert(l != NULL);
	assert(e != NULL);

	list_insert(&l->sentinal, e);
}

void list_pushback(struct list *l, struct list_elem *e)
{
	assert(l != NULL);
	assert(l->sentinal.prev != NULL);
	assert(e != NULL);

	list_insert(l->sentinal.prev, e);
}

int list_isempty(const struct list *l)
{
	assert(l != NULL);

	return (l->sentinal.prev == &l->sentinal);
}

