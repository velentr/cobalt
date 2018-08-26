#include <assert.h>
#include <string.h>

#include "dstring.h"

int main()
{
	struct dstring uut;

	dstrempty(&uut);
	assert(strcmp(dstr(&uut), "") == 0);
	assert(dstrlen(&uut) == 0);
	dstrclr(&uut);

	return 0;
}

