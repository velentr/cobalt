#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <cobalt/cobalt.h>

#define TEST_DATA	"This is a test"

/* Sanity check to ensure co_init() is working */
int main()
{
	struct cobalt *co;
	uint32_t id;
	int err = 0xdeadbeef;

	co = co_init(".", &err);
	assert(co != NULL);
	assert(err == (int)0xdeadbeef);

	id = co_add(co, TEST_DATA, strlen(TEST_DATA), "test");
	assert(id != 0);

	err = co_add_attr(co, id, "attribute", "value");
	assert(err == CO_ENOERR);

	co_free(co);

	return 0;
}
