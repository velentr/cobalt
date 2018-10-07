/*
 * Copyright (C) 2017-18 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef HTABLE_H_
#define HTABLE_H_


#include "list.h"
#include "util.h"

/* element stored in a hash table */
struct hash_elem {
	struct list_elem le;
};

/* function for hashing an element within a bucket */
typedef size_t (*hasher)(const struct hash_elem *e, size_t numbuckets);

/* hash table for associative arrays */
struct hash_table {
	struct list *buckets;
	hasher hash;
	cmp_func cmp;
	size_t len;
};

/* initialize a hash table */
void ht_init(struct hash_table *ht, struct list *buckets, size_t num,
		hasher hash, cmp_func cmp);
/* insert a new element into a hash table */
void ht_insert(struct hash_table *ht, struct hash_elem *he);
/* get an element out of a hash table */
struct hash_elem *ht_get(const struct hash_table *ht,
		const struct hash_elem *key);
/* rehash a hash table */
struct list *ht_rehash(struct hash_table *ht, struct list *buckets, size_t num);
/* count the elements in a hash table */
size_t ht_size(const struct hash_table *ht);
/* check if a hash table is empty */
int ht_isempty(const struct hash_table *ht);
/* remove an element from its hash table */
struct hash_elem *ht_remove(struct hash_elem *he);
/* check how much space is in a hash table */
size_t ht_space(const struct hash_table *ht);

/* hash a string using FNV-1 */
size_t strhash(const char *str);
size_t strhash_cont(const char *str, size_t val);

#endif /* end of include guard: HTABLE_H_ */

