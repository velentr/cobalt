#include <assert.h>
#include <string.h>

#include "dstring.h"

#define TEST_INIT "1234"
#define TEST_END  "0123456789012345678901234567890123456789"

int main()
{
	struct dstring uut;

	assert(dstrcpy(&uut, TEST_INIT) == 0);
	dstrclr(&uut);
	assert(dstrcpy(&uut, TEST_END) == 0);
	assert(strcmp(dstr(&uut), TEST_END) == 0);
	dstrclr(&uut);

	return 0;
}

