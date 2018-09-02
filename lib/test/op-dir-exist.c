#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

static void run_test(struct co_db *db, const char *dir)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, dir),
		OP_DEXIST(0),
	};
	assert(co_db_run(db, test, lengthof(test)) == CO_ENOERR);
}

int main()
{
	struct co_db db;
	struct stat st;

	co_db_init(&db);
	run_test(&db, "./dir");
	co_db_free(&db);

	assert(stat("./dir", &st) == 0);
	assert(S_ISDIR(st.st_mode));

	return 0;
}
