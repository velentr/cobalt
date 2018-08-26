/*
 * Copyright (C) 2017-18 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef LIST_H_
#define LIST_H_


#include <stddef.h>

/* Compile-time initialization of a list */
#define LIST_INIT(l) struct list l = { \
	.sentinal = { \
		.prev = &(l).sentinal, \
		.next = &(l).sentinal, \
	} \
}

/* Linked list node */
struct list_elem {
	struct list_elem *prev;
	struct list_elem *next;
};

/* Doubly-linked list structure */
struct list {
	struct list_elem sentinal;
};

/* Initialize a linked list */
void list_init(struct list *l);
/* Insert an element into a list */
void list_insert(struct list_elem *prev, struct list_elem *to_add);
/* Remove an element from a list */
struct list_elem *list_remove(struct list_elem *e);
/* Pop an element from the front of a list */
struct list_elem *list_popfront(struct list *l);
/* Pop an element from the back of a list */
struct list_elem *list_popback(struct list *l);
/* Get the number of elements in a list */
size_t list_size(const struct list *l);
/* Concatenate two lists */
void list_cat(struct list *dst, struct list *src);
/* Get the first element in a list */
struct list_elem *list_head(const struct list *l);
/* Get the last element in a list */
struct list_elem *list_tail(const struct list *l);
/* Get a beginning iterator for the list */
struct list_elem *list_begin(const struct list *l);
/* Iterate to the next element of a list */
struct list_elem *list_next(const struct list_elem *e);
/* Iterate to the previous element of a list */
struct list_elem *list_prev(const struct list_elem *e);
/* Get an ending iterator for the list */
struct list_elem *list_end(const struct list *l);
/* Push an element to the front of the list */
void list_pushfront(struct list *l, struct list_elem *e);
/* Push an element to the back of the list */
void list_pushback(struct list *l, struct list_elem *e);
/* Check if the list is empty */
int list_isempty(const struct list *l);


#endif /* end of include guard: LIST_H_ */

