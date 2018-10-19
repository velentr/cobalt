#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <cobalt/cobalt.h>

#define TEST_DATA	"This is a test"

/* Verify that we can set an attribute, then delete it; check that we cannot
 * then query the added task by that attribute.
 */
int main()
{
	struct cobalt *co;
	struct cobalt_query *q;
	uint32_t id;
	int err = 0xdeadbeef;

	co = co_init(".", &err);
	assert(co != NULL);
	assert(err == (int)0xdeadbeef);

	id = co_add(co, TEST_DATA, strlen(TEST_DATA), "test");
	assert(id != 0);

	err = co_add_attr(co, id, "attribute", "value");
	assert(err == CO_ENOERR);

	err = co_del_attr(co, id, "attribute");
	assert(err == CO_ENOERR);

	err = co_get_attr(co, "attribute", "value", &q);
	assert(err == CO_ENOERR);
	assert(q == NULL);

	co_free(co);

	return 0;
}
