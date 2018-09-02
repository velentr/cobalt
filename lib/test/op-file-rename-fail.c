#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

extern int __rename_fail;

static void run_test(struct co_db *db, const char *file)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, file),
		OP_SCPY(1, 0),
		MACRO_SCATL(0, "/old"),
		MACRO_SCATL(1, "/new"),
		OP_FRENAM(0, 1),
	};
	__rename_fail = EPERM;
	assert(co_db_run(db, test, lengthof(test)) == EPERM);
	__rename_fail = 0;
}

int main()
{
	struct co_db db;
	struct stat st;
	int fd;

	fd = open("./old", O_RDONLY | O_CREAT, 0644);
	assert(fd != -1);
	close(fd);

	co_db_init(&db);
	run_test(&db, ".");
	co_db_free(&db);

	assert(stat("./new", &st) == -1);
	assert(errno == ENOENT);

	assert(stat("./old", &st) == 0);
	assert(S_ISREG(st.st_mode));

	return 0;
}
