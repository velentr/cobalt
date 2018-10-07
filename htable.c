/*
 * Copyright (C) 2017-18 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "htable.h"
#include "list.h"
#include "util.h"

void ht_init(struct hash_table *ht, struct list *buckets, size_t num,
		hasher hash, cmp_func cmp)
{
	size_t i;

	assert(ht != NULL);
	assert(buckets != NULL);
	assert(num > 0);
	assert(hash != NULL);
	assert(cmp != NULL);

	ht->buckets = buckets;
	ht->hash = hash;
	ht->cmp = cmp;
	ht->len = num;

	for (i = 0; i < num; i++)
		list_init(buckets + i);
}

void ht_insert(struct hash_table *ht, struct hash_elem *he)
{
	size_t hval;

	assert(ht != NULL);
	assert(he != NULL);
	assert(ht_get(ht, he) == NULL);

	hval = ht->hash(he, ht->len);
	assert(hval < ht->len);

	list_pushfront(&ht->buckets[hval], &he->le);
}

struct hash_elem *ht_get(const struct hash_table *ht,
		const struct hash_elem *key)
{
	size_t hval;
	struct list *bucket;
	struct hash_elem *rc;
	struct list_elem *le;
	cmp_func cmp;

	assert(ht != NULL);
	assert(key != NULL);

	/* find the appropriate bucket to search through */
	hval = ht->hash(key, ht->len);
	assert(hval < ht->len);
	bucket = &ht->buckets[hval];
	cmp = ht->cmp;

	/* return NULL if the key isn't found */
	rc = NULL;

	list_foreach(le, bucket) {
		/* get hash entry corresponding to the current list iterator */
		struct hash_elem *val = containerof(le, struct hash_elem, le);

		/* if the value matches the key, then return it */
		if (cmp(val, key) == 0) {
			rc = val;
			break;
		}
	}

	return rc;
}

struct list *ht_rehash(struct hash_table *ht, struct list *buckets, size_t num)
{
	struct list *old_buckets;
	struct list *b;
	struct list_elem *le;
	struct hash_elem *val;
	size_t old_num;
	size_t i;

	assert(ht != NULL);
	assert(buckets != NULL);
	assert(num > 0);

	/* store the new buckets array in the hash table */
	old_buckets = ht->buckets;
	old_num = ht->len;
	ht_init(ht, buckets, num, ht->hash, ht->cmp);

	/* take all elements out of the old buckets and put them in the new */
	for (i = 0; i < old_num; i++) {
		b = old_buckets + i;
		while (!list_isempty(b)) {
			le = list_popfront(b);
			val = containerof(le, struct hash_elem, le);
			ht_insert(ht, val);
		}
	}

	return old_buckets;
}

size_t ht_size(const struct hash_table *ht)
{
	size_t nelems;
	size_t len;
	struct list *buckets;
	size_t i;

	assert(ht != NULL);

	nelems = 0;
	len = ht->len;
	buckets = ht->buckets;

	for (i = 0; i < len; i++)
		nelems += list_size(buckets + i);

	return nelems;
}

int ht_isempty(const struct hash_table *ht)
{
	size_t i;
	size_t len;
	struct list *buckets;

	assert(ht != NULL);

	buckets = ht->buckets;
	len = ht->len;

	/* loop through each buckets, checking whether or not it is empty */
	for (i = 0; i < len; i++) {
		if (!list_isempty(buckets + i))
			return 0;
	}

	return 1;
}

struct hash_elem *ht_remove(struct hash_elem *he)
{
	assert(he != NULL);

	(void)list_remove(&he->le);

	return he;
}

size_t ht_space(const struct hash_table *ht)
{
	assert(ht != NULL);

	return ht->len;
}

#define HASH_PRIME \
	(sizeof(size_t) == sizeof(uint32_t) ? 16777619 : 1099511628211)
#define HASH_BASIS \
	(sizeof(size_t) == sizeof(uint32_t) ? 0x811c9dc5 : 0xcbf29ce484222325)

size_t strhash(const char *str)
{
	return strhash_cont(str, HASH_BASIS);
}

size_t strhash_cont(const char *str, size_t val)
{
	while (*str != '\0') {
		val = (val * HASH_PRIME) ^ *str;
		str++;
	}

	return val;
}

