#include <assert.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

extern int __mkdir_fail;

static void run_test(struct co_db *db, const char *dir)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, dir),
		OP_DEXIST(0),
	};
	__mkdir_fail = EACCES;
	assert(co_db_run(db, test, lengthof(test)) == EACCES);
}

int main()
{
	struct co_db db;

	co_db_init(&db);
	run_test(&db, "./dir");
	co_db_free(&db);

	return 0;
}
