#include <assert.h>

#include <cobalt/cobalt.h>

/* Sanity check to ensure co_open() is working */
int main()
{
	struct cobalt *co;
	int err = 0xdeadbeef;

	co = co_open(".", &err);
	assert(co != NULL);
	assert(err == (int)0xdeadbeef);
	co_free(co);

	return 0;
}
