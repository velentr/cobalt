#include <assert.h>
#include <errno.h>

#include <cobalt/cobalt.h>

extern int __malloc_fail;

/* Check for malloc() failure when allocating the database handle */
int main()
{
	struct cobalt *co;
	int err = 0xdeadbeef;

	__malloc_fail = EIO;
	co = co_open(".", &err);
	__malloc_fail = 0;

	assert(co == NULL);
	assert(err == EIO);
	co_free(co);

	return 0;
}
