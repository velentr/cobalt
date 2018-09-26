#include <assert.h>
#include <errno.h>

#include <cobalt/cobalt.h>

extern int __malloc_fail;
extern int __malloc_fail_count;

#define LONG_TESTPATH "this/is/a/long/directory/path/name/to/cause/a/malloc"

/* Check for malloc() failure when allocating memory for the dstring */
int main()
{
	struct cobalt *co;
	int err = 0xdeadbeef;

	__malloc_fail = EIO;
	__malloc_fail_count = 1;
	co = co_open(LONG_TESTPATH, &err);
	__malloc_fail = 0;

	assert(co == NULL);
	assert(err == EIO);
	co_free(co);

	return 0;
}
