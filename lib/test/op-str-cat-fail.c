#include <assert.h>
#include <errno.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

#define TEST_STRING "0123456789012345678901234567890123456789"

extern int __malloc_fail;
extern int __realloc_fail;

static void run_test(struct co_db *db, const char *dir)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, TEST_STRING),
	};
	(void)dir;
	__malloc_fail = 1;
	__realloc_fail = 1;
	assert(co_db_run(db, test, lengthof(test)) == ENOMEM);
	__malloc_fail = 0;
	__realloc_fail = 0;
}

int main()
{
	struct co_db db;

	co_db_init(&db);
	run_test(&db, ".");
	co_db_free(&db);

	return 0;
}
