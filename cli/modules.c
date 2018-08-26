/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <string.h>

#include "list.h"
#include "modules.h"

LIST_INIT(commands);

struct module *module_get(const char *name)
{
	struct list_elem *le;
	struct module *m;

	for (le = list_begin(&commands); le != list_end(&commands);
			le = list_next(le)) {
		m = moduleof(le);
		if (strcmp(m->name, name) == 0)
			return m;
	}

	return NULL;
}

