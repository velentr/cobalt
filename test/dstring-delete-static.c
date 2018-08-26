#include <assert.h>
#include <string.h>

#include "dstring.h"

#define TEST_STRING "0123456789"
#define TEST_RESULT "0123"

int main()
{
	struct dstring uut = DSTR_INIT(TEST_STRING);

	dstrdel(&uut, 6);
	assert(strcmp(dstr(&uut), TEST_RESULT) == 0);
	dstrclr(&uut);

	return 0;
}

