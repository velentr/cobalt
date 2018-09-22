#include <assert.h>
#include <string.h>

#include "dstring.h"

#define TEST_STRING "0123456789"
#define TEST_RESULT "0123"

int main()
{
	struct dstring uut;

	assert(dstrcpy(&uut, TEST_STRING) == 0);
	dstrdel(&uut, 6);
	assert(strcmp(dstr(&uut), TEST_RESULT) == 0);
	dstrclr(&uut);

	return 0;
}

