#include <assert.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

static void run_test(struct co_db *db, const char *dir)
{
	struct co_db_op test[] = {
		OP_NOP,
	};
	(void)dir;
	assert(co_db_run(db, test, lengthof(test)) == CO_ENOERR);
}

int main(int argc, const char * const argv[])
{
	struct co_db db;
	const char *dir;

	if (argc == 1)
		dir = ".";
	else
		dir = argv[1];
	co_db_init(&db);
	run_test(&db, dir);
	co_db_free(&db);

	return 0;
}
