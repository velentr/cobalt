#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

extern int __unlink_fail;

static void run_test(struct co_db *db, const char *file)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, file),
		OP_SCPY(1, 0),
		MACRO_SCATL(0, "/target"),
		MACRO_SCATL(1, "/link"),
		OP_LDEL(0, 1),
	};
	__unlink_fail = EPERM;
	assert(co_db_run(db, test, lengthof(test)) == EPERM);
}

int main()
{
	struct co_db db;
	struct stat st;

	assert(symlink(".", "./link") == 0);

	co_db_init(&db);
	run_test(&db, ".");
	co_db_free(&db);

	assert(lstat("./link", &st) == 0);
	assert(S_ISLNK(st.st_mode));

	return 0;
}
