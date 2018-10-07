/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

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
		/* TODO check for error */
		conf_adds(sname, slen, kname, klen, sval, p - sval - 1);
	}

	action setbooltrue {
		/* TODO check for error */
		conf_addb(sname, slen, kname, klen, 1);
	}
	action setboolfalse {
		/* TODO check for error */
		conf_addb(sname, slen, kname, klen, 0);
	}

	action setlong {
		/* TODO check for error */
		conf_addl(sname, slen, kname, klen, strtol(lstart, NULL, 0));
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

/* XXX move open/fstat/mmap to common code */
static int config_open(const char *file, const char **buf, size_t *len)
{
	struct stat st;
	int fd;
	int rc;

	fd = open(file, O_RDONLY);
	if (fd == -1)
		return errno;

	rc = fstat(fd, &st);
	if (rc == -1) {
		rc = errno;
		close(fd);
		return rc;
	}

	*len = st.st_size;
	if (*len == 0) {
		rc = errno;
		close(fd);
		return rc;
	}

	*buf = mmap(NULL, *len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (*buf == MAP_FAILED) {
		rc = errno;
		close(fd);
		return rc;
	}
	close(fd);

	return 0;
}

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

	rc = config_open(file, &buf, &len);
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
		return EINVAL;
	else
		return 0;
}

