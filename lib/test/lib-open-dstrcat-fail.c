#include <assert.h>
#include <errno.h>

#include <cobalt/cobalt.h>

extern int __realloc_fail;

#define LONG_TESTPATH "this/is/a/long/directory/path/name/to/cause/a/realloc" \
	"test/fail"

/* Check for realloc() failure when appending to the dstring in co_open() */
int main()
{
	struct cobalt *co;
	int err = 0xdeadbeef;

	__realloc_fail = EIO;
	co = co_open(LONG_TESTPATH, &err);
	__realloc_fail = 0;

	assert(co == NULL);
	assert(err == EIO);
	co_free(co);

	return 0;
}
