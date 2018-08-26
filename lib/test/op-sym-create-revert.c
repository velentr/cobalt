#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "dstring.h"
#include "struct.h"
#include "util.h"

static void run_test(struct co_db *db, const char *file)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, file),
		OP_SCPY(1, 0),
		MACRO_SCATL(0, "/target"),
		MACRO_SCATL(1, "/link"),
		OP_LCREAT(0, 1),
		OP_ABORT,
	};
	assert(co_db_run(db, test, lengthof(test)) == CO_EABORT);
}

int main(int argc, const char * const argv[])
{
	struct dstring root = DSTR_EMPTY;
	struct co_db db;
	struct stat st;

	if (argc == 1)
		assert(dstrcat(&root, ".") == CO_ENOERR);
	else
		assert(dstrcat(&root, argv[1]) == CO_ENOERR);
	co_db_init(&db);
	run_test(&db, dstr(&root));
	co_db_free(&db);

	assert(dstrcat(&root, "/link") == 0);
	assert(lstat(dstr(&root), &st) == -1);
	assert(errno == ENOENT);

	dstrclr(&root);

	return 0;
}
