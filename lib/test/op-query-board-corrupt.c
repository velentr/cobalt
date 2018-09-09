#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

static void run_test(struct co_db *db, const char *dir)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, dir),
		OP_QILD(0),
		OP_QIRD,
		OP_QBD,
	};
	assert(co_db_run(db, test, lengthof(test)) == CO_ECORRUPT);
}

int main()
{
	struct co_db db;

	assert(mkdir("./dir", 0755) == 0);
	assert(mkdir("./dir/00000001", 0755) == 0);
	assert(mkdir("./dir/00000001/attr", 0755) == 0);
	assert(symlink("test", "./dir/00000001/attr/board") == 0);

	co_db_init(&db);
	run_test(&db, "./dir");

	assert(list_isempty(&db.query));

	co_db_free(&db);
	return 0;
}
