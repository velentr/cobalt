#include <assert.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

extern int __open_fail;

static void run_test(struct co_db *db, const char *file)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, file),
		OP_FCREAT(0, "test", strlen("test")),
	};
	__open_fail = ENFILE;
	assert(co_db_run(db, test, lengthof(test)) == ENFILE);
	__open_fail = 0;
}

int main()
{
	struct co_db db;
	struct stat st;

	co_db_init(&db);
	run_test(&db, "./file");
	co_db_free(&db);

	assert(stat("./file", &st) == -1);
	assert(errno == ENOENT);

	return 0;
}
