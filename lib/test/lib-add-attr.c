#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <cobalt/cobalt.h>

#define TEST_DATA	"This is a test"

/* Verify that we can set an attribute and query the added task by that
 * attribute.
 */
int main()
{
	struct cobalt *co;
	struct cobalt_query *q;
	const char *data;
	size_t len;
	uint32_t id;
	int err = 0xdeadbeef;

	co = co_init(".", &err);
	assert(co != NULL);
	assert(err == (int)0xdeadbeef);

	id = co_add(co, TEST_DATA, strlen(TEST_DATA), "test");
	assert(id != 0);

	err = co_add_attr(co, id, "attribute", "value");
	assert(err == CO_ENOERR);

	err = co_get_attr(co, "attribute", "value", &q);
	assert(err == CO_ENOERR);

	assert(co_query_getid(q) == id);
	assert(strcmp(co_query_getboard(q), "test") == 0);
	co_query_getdata(q, &data, &len);
	assert(len == strlen(TEST_DATA));
	assert(memcmp(data, TEST_DATA, len) == 0);

	assert(co_query_getnext(co, q) == NULL);

	co_free(co);

	return 0;
}
