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
		OP_QMAP,
		OP_ABORT,
	};
	assert(co_db_run(db, test, lengthof(test)) == CO_EABORT);
}

int main()
{
	struct co_db db;
	int fd;

	assert(mkdir("./dir", 0755) == 0);
	assert(mkdir("./dir/00000001", 0755) == 0);
	assert((fd = open("./dir/00000001/data", O_WRONLY | O_CREAT, 0644)) != -1);
	assert(write(fd, "test", strlen("test")) == strlen("test"));
	close(fd);

	co_db_init(&db);
	run_test(&db, "./dir");

	assert(list_isempty(&db.query));

	co_db_free(&db);
	return 0;
}
