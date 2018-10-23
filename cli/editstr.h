/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef EDITSTR_H_
#define EDITSTR_H_


#include <stdio.h>
#include <stdlib.h>

#include "dstring.h"

#define EDITSTR_TMPFILE	"/tmp/cobalt_XXXXXX"

/* dynamic string created by an external text editor */
struct editstr {
	char fname[sizeof(EDITSTR_TMPFILE)];
	const char *data;
	size_t len;
	int fd;
};

/* create a new editor string */
int editstr_create(struct dstring *s, const char *template, size_t len);

/* print the editstring */
int editstr_write(struct dstring *s, const struct editstr *es);


#endif /* end of include guard: EDITSTR_H_ */

