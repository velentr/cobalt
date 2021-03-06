#include <assert.h>
#include <string.h>

#include "dstring.h"

#define TEST_PREFIX "1234"
#define TEST_SUFFIX "5678"

int main()
{
	struct dstring uut;

	assert(dstrcpy(&uut, TEST_PREFIX) == 0);
	assert(dstrgrow(&uut, strlen(TEST_SUFFIX)) == 0);
	assert(strcmp(dstr(&uut), TEST_PREFIX) == 0);
	assert(dstrlen(&uut) == strlen(TEST_PREFIX));
	dstrclr(&uut);

	return 0;
}

