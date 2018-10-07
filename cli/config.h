/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef CONFIG_H_
#define CONFIG_H_


#include <stdlib.h>

/* initialize static memory used by the config subsystem */
int conf_init(void);
/* free all memory used by the config subsystem */
void conf_free(void);
/* parse a config file and add its definititions to the current config */
int conf_parse(const char *file);

/* add a new value to the config, replacing any existing value */
int conf_adds(const char *section, size_t slen, const char *key, size_t klen,
		const char *value, size_t vlen);
int conf_addl(const char *section, size_t slen, const char *key, size_t klen,
		long value);
int conf_addb(const char *section, size_t slen, const char *key, size_t klen,
		int value);

/* access configuration value 'key' in section 'section'; the 'def' is the
 * default value returned if the value does not exist or is of the wrong type
 */
const char *conf_gets(const char *section, const char *key, const char *def);
long conf_getl(const char *section, const char *key, long def);
int conf_getb(const char *section, const char *key, int def);


#endif /* end of include guard: CONFIG_H_ */

