/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "argparse.h"
#include "config.h"
#include "htable.h"
#include "modules.h"
#include "util.h"

/* initial number of buckets in the hash table; should be a prime number large
 * enough that it won't be more than 80% full, but small enough that it doesn't
 * waste a lot of space
 */
#define CONF_INIT_SIZE	241

struct conf_entry {
	/* hash_elem must be first so we can cast the pointer to conf_entry */
	struct hash_elem he;
	const char *section;
	const char *key;
	enum { CONF_STRING, CONF_LONG, CONF_BOOL, } type;
	union {
		const char *s;
		long l;
		int b;
	};
};

static struct list buckets[CONF_INIT_SIZE];
static struct hash_table config;

static size_t conf_hash(const struct conf_entry *c, size_t nbuckets)
{
	return strhash_cont(c->key, strhash(c->section)) % nbuckets;
}

static int conf_cmp(const struct conf_entry *c0, const struct conf_entry *c1)
{
	int rc;

	rc = strcmp(c0->section, c1->section);
	if (rc == 0)
		return strcmp(c0->key, c1->key);
	else
		return rc;
}

int conf_init(void)
{
	ht_init(&config, buckets, lengthof(buckets), (hasher) conf_hash,
			(cmp_func) conf_cmp);

	/* in the future, the 'buckets' array may need to be dynamically
	 * allocated if it is expected to grow, so all callers of this function
	 * should check for errors; right now, this always succeeds
	 */
	return 0;
}

void conf_free(void)
{
	struct conf_entry *ent;
	size_t i;

	for (i = 0; i < lengthof(buckets); i++) {
		while (!list_isempty(buckets + i)) {
			/* XXX better way to free hash elements? */
			ent = (struct conf_entry *)list_popfront(buckets + i);
			free(ent);
		}
	}
}

static void conf_setup(struct conf_entry *ent, const char *section, size_t slen,
		const char *key, size_t klen)
{
	char *s;

	/* store data from each string into the structure */
	s = (char *)(ent + 1);
	memcpy(s, section, slen);
	s[slen] = '\0';
	ent->section = s;

	s += slen + 1;
	memcpy(s, key, klen);
	s[klen] = '\0';
	ent->key = s;
}

int conf_adds(const char *section, size_t slen, const char *key, size_t klen,
		const char *value, size_t vlen)
{
	struct conf_entry *ent;
	char *s;

	/* make sure to allocate memory for the 3 nil bytes */
	ent = malloc(sizeof(*ent) + slen + klen + vlen + 3);
	if (ent == NULL)
		return errno;

	conf_setup(ent, section, slen, key, klen);

	s = ((char *)(ent + 1)) + slen + klen + 2;
	memcpy(s, value, vlen);
	s[vlen] = '\0';
	ent->s = s;
	ent->type = CONF_STRING;

	ht_insert(&config, &ent->he);

	return 0;
}

int conf_addl(const char *section, size_t slen, const char *key, size_t klen,
		long value)
{
	struct conf_entry *ent;

	/* make sure to allocate memory for the 2 nil bytes */
	ent = malloc(sizeof(*ent) + slen + klen + 2);
	if (ent == NULL)
		return errno;

	conf_setup(ent, section, slen, key, klen);

	ent->l = value;
	ent->type = CONF_LONG;

	ht_insert(&config, &ent->he);

	return 0;
}

int conf_addb(const char *section, size_t slen, const char *key, size_t klen,
		int value)
{
	struct conf_entry *ent;

	/* make sure to allocate memory for the 2 nil bytes */
	ent = malloc(sizeof(*ent) + slen + klen + 2);
	if (ent == NULL)
		return errno;

	conf_setup(ent, section, slen, key, klen);

	ent->b = value;
	ent->type = CONF_BOOL;

	ht_insert(&config, &ent->he);

	return 0;
}

const char *conf_gets(const char *section, const char *key, const char *def)
{
	struct conf_entry k;
	struct conf_entry *ent;

	k.section = section;
	k.key = key;
	ent = (struct conf_entry *)ht_get(&config, &k.he);

	if (ent == NULL || ent->type != CONF_STRING)
		return def;
	else
		return ent->s;
}

long conf_getl(const char *section, const char *key, long def)
{
	struct conf_entry k;
	struct conf_entry *ent;

	k.section = section;
	k.key = key;
	ent = (struct conf_entry *)ht_get(&config, &k.he);

	if (ent == NULL || ent->type != CONF_LONG)
		return def;
	else
		return ent->l;
}

int conf_getb(const char *section, const char *key, int def)
{
	struct conf_entry k;
	struct conf_entry *ent;

	k.section = section;
	k.key = key;
	ent = (struct conf_entry *)ht_get(&config, &k.he);

	if (ent == NULL || ent->type != CONF_BOOL)
		return def;
	else
		return ent->b;
}

static struct arg filename = {
	.name = "filename",
	.desc = "name of config file to parse",
	.type = ARG_BARE,
	.exclude = { NULL }
};

static int config_main(void)
{
	int rc;

	if (!filename.valid) {
		eprint("config filename required\n");
		return EXIT_FAILURE;
	}

	rc = conf_init();
	if (rc != 0) {
		eprint("%s\n", strerror(rc));
		return EXIT_FAILURE;
	}

	rc = conf_parse(filename.bare.value);
	if (rc != 0) {
		eprint("%s: %s\n", filename.bare.value, strerror(rc));
		rc = EXIT_FAILURE;
	} else {
		rc = EXIT_SUCCESS;
	}

	conf_free();

	return rc;
}

static struct module config_module = {
	.name = "config",
	.desc = "check configuration file syntax",
	.main = config_main,
	.args = { &filename, NULL }
};

MODULE_INIT(config_module)

