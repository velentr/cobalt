/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <string.h>

#include <cobalt/cobalt_error.h>

#include "struct.h"

static const char *co_strerror_priv(int errnum)
{
	const char *ret;

	switch (errnum) {
	case CO_ENOERR:
		ret = "No error";
		break;
	case CO_EIDINUSE:
		ret = "Object ID already exists";
		break;
	case CO_EINCOMPLETE:
		ret = "Incomplete write detected";
		break;
	case CO_EABORT:
		ret = "Operation aborted";
		break;
	case CO_ECORRUPT:
		ret = "Database corruption detected";
		break;
	default:
		ret = "Unknown error";
		break;
	}

	assert(strlen(ret) < 1024);

	return ret;
}

const char *co_strerror(struct cobalt *co)
{
	const char *errstr;

	if (co->err > 0) {
		strerror_r(co->err, co->errstr, sizeof(co->errstr));
	} else {
		errstr = co_strerror_priv(co->err);
		assert(strlen(errstr) < sizeof(co->errstr));
		strcpy(co->errstr, errstr);
	}

	return co->errstr;
}

