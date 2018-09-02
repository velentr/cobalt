#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

extern int __symlink_fail;

static void run_test(struct co_db *db, const char *file)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, file),
		OP_SCPY(1, 0),
		MACRO_SCATL(0, "/target"),
		MACRO_SCATL(1, "/link"),
		OP_LCREAT(0, 1),
	};
	__symlink_fail = EACCES;
	assert(co_db_run(db, test, lengthof(test)) == EACCES);
	__symlink_fail = 0;
}

int main()
{
	struct co_db db;
	struct stat st;

	co_db_init(&db);
	run_test(&db, ".");
	co_db_free(&db);

	assert(lstat("./link", &st) == -1);
	assert(errno == ENOENT);

	return 0;
}
