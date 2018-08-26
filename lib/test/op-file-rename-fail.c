#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "dstring.h"
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

int main(int argc, const char * const argv[])
{
	struct dstring file = DSTR_EMPTY;
	struct co_db db;
	struct stat st;
	int fd;

	if (argc == 1)
		assert(dstrcat(&file, ".") == CO_ENOERR);
	else
		assert(dstrcat(&file, argv[1]) == CO_ENOERR);

	assert(dstrcat(&file, "/old") == CO_ENOERR);
	fd = open(dstr(&file), O_RDONLY | O_CREAT, 0644);
	assert(fd != -1);
	close(fd);
	dstrdel(&file, 4);

	co_db_init(&db);
	run_test(&db, dstr(&file));
	co_db_free(&db);

	assert(dstrcat(&file, "/new") == 0);
	assert(stat(dstr(&file), &st) == -1);
	assert(errno == ENOENT);
	dstrdel(&file, 4);

	assert(dstrcat(&file, "/old") == 0);
	assert(stat(dstr(&file), &st) == 0);
	assert(S_ISREG(st.st_mode));

	dstrclr(&file);

	return 0;
}
