#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <cobalt/cobalt.h>

#include "struct.h"

/* Make sure seeding the RNG using the environment is working */
int main()
{
	struct cobalt *co;
	uint32_t id;
	int err;

	setenv("COBALT_SEED", "0", 1);

	co = co_init(".", &err);
	assert(co != NULL);

	id = co_add(co, "test", strlen("test"), "test");
	assert(id == 0);
	assert(co->err == EAGAIN);

	co_free(co);

	return 0;
}
