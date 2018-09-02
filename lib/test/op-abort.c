#include <assert.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

static void run_test(struct co_db *db, const char *dir)
{
	struct co_db_op test[] = {
		OP_ABORT,
	};
	(void)dir;
	assert(co_db_run(db, test, lengthof(test)) == CO_EABORT);
}

int main()
{
	struct co_db db;

	co_db_init(&db);
	run_test(&db, ".");
	co_db_free(&db);

	return 0;
}
