#include <assert.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "dstring.h"
#include "struct.h"
#include "util.h"

extern int __write_fail;

static void run_test(struct co_db *db, const char *file)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, file),
		OP_FCREAT(0, "test", strlen("test")),
	};
	__write_fail = EIO;
	assert(co_db_run(db, test, lengthof(test)) == EIO);
	__write_fail = 0;
}

int main(int argc, const char * const argv[])
{
	struct dstring file = DSTR_EMPTY;
	struct co_db db;
	struct stat st;

	if (argc == 1)
		assert(dstrcat(&file, ".") == CO_ENOERR);
	else
		assert(dstrcat(&file, argv[1]) == CO_ENOERR);
	assert(dstrcat(&file, "/file") == CO_ENOERR);
	co_db_init(&db);
	run_test(&db, dstr(&file));
	co_db_free(&db);

	assert(stat(dstr(&file), &st) == -1);
	assert(errno == ENOENT);

	dstrclr(&file);

	return 0;
}
