#include <assert.h>
#include <string.h>

#include "htable.h"
#include "list.h"
#include "util.h"

struct hashtest {
	struct hash_elem he;
	const char *key;
};

static size_t hash(const struct hashtest *e, size_t numbuckets)
{
	return strhash(e->key) % numbuckets;
}

static int cmp(const struct hashtest *e0, const struct hashtest *e1)
{
	return strcmp(e0->key, e1->key);
}

struct list buckets[10];

int main()
{
	struct hash_table haystack;
	struct hashtest needle = { .key = "test" };
	struct hashtest uut = { .key = "test" };

	ht_init(&haystack, buckets, lengthof(buckets), (void *)&hash,
			(void *)&cmp);

	assert(ht_isempty(&haystack));
	assert(ht_space(&haystack) == lengthof(buckets));

	ht_insert(&haystack, &uut.he);

	assert(ht_get(&haystack, &needle.he) == &uut.he);
	assert(ht_size(&haystack) == 1);
	assert(!ht_isempty(&haystack));

	return 0;
}
