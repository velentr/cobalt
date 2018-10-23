/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

%%{
	machine config_parse;

	write data noerror nofinal noentry;

	action startkey { kname = p; }
	action startsec { sname = p; }
	action endkey { klen = (p - kname); }
	action endsec { slen = (p - sname); }
	action startsval { sval = p + 1; }
	action startl { lstart = p; }

	action setstr {
		rc = conf_adds(sname, slen, kname, klen, sval, p - sval - 1);
		if (rc != 0)
			goto conf_error;
	}

	action setbooltrue {
		rc = conf_addb(sname, slen, kname, klen, 1);
		if (rc != 0)
			goto conf_error;
	}
	action setboolfalse {
		rc = conf_addb(sname, slen, kname, klen, 0);
		if (rc != 0)
			goto conf_error;
	}

	action setlong {
		rc = conf_addl(sname, slen, kname, klen,
				strtol(lstart, NULL, 0));
		if (rc != 0)
			goto conf_error;
	}

	nl = [\r\n];
	wsp = [ \t\v]+;
	cmt = '#' [^\r\n]*;

	eol = wsp? cmt? nl;

	atom = [a-zA-Z0-9][\-a-zA-Z0-9_/]*;
	sec = '[' (atom >startsec %endsec) ']' eol;

	bval = ("true" %setbooltrue) | ("false" %setboolfalse);
	lval = ("0x"? [0-9]+) >startl %setlong;
	sval = ('"' [^\0"\n\r#]* '"') >startsval %setstr;
	val = bval | lval | sval;
	kv = wsp? (atom >startkey %endkey) wsp? '=' wsp? val eol;

	main := eol* (sec (eol | kv)*)*;
}%%

int conf_parse(const char *file)
{
	const char *buf;
	const char *p, *pe, *eof;
	const char *sname, *kname;
	const char *sval;
	const char *lstart;
	size_t slen, klen;
	size_t len;
	int cs;
	int rc;

	rc = fmap(file, &buf, &len);
	if (rc != 0)
		return rc;
	else if (len == 0)
		return 0;

	p = buf;
	pe = p + len;
	eof = pe;

	%% write init;
	%% write exec;

	if (cs == %%{ write error; }%%)
		rc = EINVAL;
	else
		rc = 0;

conf_error:
	munmap((void *)buf, len);

	return rc;

}

