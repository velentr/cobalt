#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <cobalt/cobalt.h>

#include "struct.h"

/* Make sure hidden filenames are invalid attribute values */
int main()
{
	struct cobalt *co;
	int err = 0xdeadbeef;
	uint32_t id;

	co = co_init(".", &err);
	assert(co != NULL);

	id = co_add(co, "test", 4, ".test");
	assert(id == 0);
	assert(co->err == EINVAL);

	co_free(co);

	return 0;
}
