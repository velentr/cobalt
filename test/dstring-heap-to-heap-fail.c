#include <assert.h>
#include <errno.h>
#include <string.h>

#include "dstring.h"

#define TEST_PREFIX "1234567890123456789012345678901234567890"
#define TEST_SUFFIX "1234567890123456789012345678901234567890"

extern int __realloc_fail;

int main()
{
	struct dstring uut;

	assert(dstrcpy(&uut, TEST_PREFIX) == 0);
	__realloc_fail = 1;
	assert(dstrcat(&uut, TEST_SUFFIX) == ENOMEM);
	__realloc_fail = 0;
	assert(strcmp(dstr(&uut), TEST_PREFIX) == 0);
	dstrclr(&uut);

	return 0;
}

