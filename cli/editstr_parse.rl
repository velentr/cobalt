/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <errno.h>

#include "dstring.h"
#include "editstr.h"

%%{
	machine editstr_write;

	write data noerror nofinal noentry;

	action startline {
		startl = p;
	}

	action endline {
		endl = p;
	}

	action writeline {
		rc = dstrgrow(s, endl - startl + 1);
		if (rc != 0)
			return rc;

		/* these cat()s can't fail if the grow succeeded */
		rc = dstrcatl(s, startl, endl - startl);
		assert(rc == 0);
		rc = dstrcat(s, "\n");
		assert(rc == 0);

		empty = 0;
	}

	nl = [\r\n];
	wsp = [ \t\v]+;
	cmt = '#' [^\r\n]*;

	eol = wsp? cmt? nl;
	line = [^#\r\n]* [^#\r\n \t\v];

	main := eol*
		( line >startline %endline eol ) %writeline
		( ( line? >startline %endline eol ) %writeline )*;
}%%

int editstr_write(struct dstring *s, const struct editstr *es)
{
	const char *p, *pe, *eof;
	const char *startl, *endl;
	int cs;
	int empty;
	int rc;

	p = es->data;
	pe = p + es->len;
	eof = pe;

	empty = 1;

	%% write init;
	%% write exec;

	if (empty)
		return ECANCELED;
	else
		return 0;
}

