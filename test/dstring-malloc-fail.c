#include <assert.h>
#include <errno.h>
#include <string.h>

#include "dstring.h"

#define TEST_STRING "1234567890123456789012345678901234567890"

extern int __malloc_fail;

int main()
{
	struct dstring uut;

	__malloc_fail = 1;
	assert(dstrcpy(&uut, TEST_STRING) == ENOMEM);
	__malloc_fail = 0;

	return 0;
}

