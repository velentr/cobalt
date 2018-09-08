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
	};
	assert(co_db_run(db, test, lengthof(test)) == CO_ENOERR);
}

int main()
{
	struct co_db db;
	struct cobalt_query *q;
	int fd;

	assert(mkdir("./dir", 0755) == 0);
	assert(mkdir("./dir/00000001", 0755) == 0);
	assert((fd = open("./dir/00000001/data", O_WRONLY | O_CREAT, 0644)) != -1);
	assert(write(fd, "test", strlen("test")) == strlen("test"));
	close(fd);

	co_db_init(&db);
	run_test(&db, "./dir");

	assert(list_size(&db.query) == 1);
	q = containerof(list_head(&db.query), struct cobalt_query, le);
	assert(memcmp(q->data, "test", strlen("test")) == 0);
	assert(q->len == strlen("test"));

	co_db_free(&db);
	return 0;
}
