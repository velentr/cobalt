#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "struct.h"
#include "util.h"

#define TEST_CONTENTS "this is a test"

static void run_test(struct co_db *db, const char *file)
{
	struct co_db_op test[] = {
		MACRO_SCATL(0, file),
		OP_FCREAT(0, TEST_CONTENTS, strlen(TEST_CONTENTS)),
	};
	assert(co_db_run(db, test, lengthof(test)) == CO_ENOERR);
}

int main()
{
	struct co_db db;
	struct stat st;
	int fd;
	char buf[BUFSIZ];

	co_db_init(&db);
	run_test(&db, "./file");
	co_db_free(&db);

	assert(stat("./file", &st) == 0);
	assert(S_ISREG(st.st_mode));

	fd = open("./file", O_RDONLY);
	assert(fd != -1);
	assert(read(fd, buf, sizeof(buf)) == strlen(TEST_CONTENTS));
	close(fd);
	buf[strlen(TEST_CONTENTS)] = '\0';

	assert(strcmp(buf, TEST_CONTENTS) == 0);

	return 0;
}
