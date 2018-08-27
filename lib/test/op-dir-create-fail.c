#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "dstring.h"
#include "struct.h"
#include "util.h"

static void run_test(struct co_db *db, const char *dir)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, dir),
		OP_DCREAT(0),
	};
	assert(co_db_run(db, test, lengthof(test)) != CO_ENOERR);
}

int main(int argc, const char * const argv[])
{
	struct dstring dir = DSTR_EMPTY;
	struct co_db db;

	if (argc == 1)
		assert(dstrcat(&dir, ".") == CO_ENOERR);
	else
		assert(dstrcat(&dir, argv[1]) == CO_ENOERR);
	assert(dstrcat(&dir, "/dir") == CO_ENOERR);
	assert(mkdir(dstr(&dir), 0755) == 0);
	co_db_init(&db);
	run_test(&db, dstr(&dir));
	co_db_free(&db);
	dstrclr(&dir);

	return 0;
}
