#include <assert.h>
#include <string.h>

#include "dstring.h"

#define TEST_STRING "1234"

int main()
{
	struct dstring uut;

	assert(dstrcpy(&uut, TEST_STRING) == 0);
	assert(strcmp(dstr(&uut), TEST_STRING) == 0);
	assert(dstrlen(&uut) == strlen(TEST_STRING));
	dstrclr(&uut);

	return 0;
}

