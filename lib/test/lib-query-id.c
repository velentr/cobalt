#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <cobalt/cobalt.h>

/* Make sure we can query a single task by its ID */
int main()
{
	struct cobalt *co;
	struct cobalt_query *q;
	const char *data;
	size_t len;
	int err = 0xdeadbeef;
	uint32_t id;

	co = co_init(".", &err);
	assert(co != NULL);
	assert(err == (int)0xdeadbeef);

	id = co_add(co, "test", 4, "test");
	assert(id != 0);

	err = co_get_task(co, id, &q);
	assert(err == CO_ENOERR);
	assert(co_query_getid(q) == id);
	assert(strcmp(co_query_getboard(q), "test") == 0);
	co_query_getdata(q, &data, &len);
	assert(len == 4);
	assert(memcmp(data, "test", len) == 0);
	assert(co_query_getnext(co, q) == NULL);

	co_free(co);

	return 0;
}
