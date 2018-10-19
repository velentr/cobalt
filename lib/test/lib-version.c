#include <assert.h>

#include <cobalt/cobalt.h>

/* Make sure we can check the version */
int main()
{
	int major, minor, patch;

	co_version(&major, &minor, &patch);
	assert(major == CO_MAJOR);
	assert(minor == CO_MINOR);
	assert(patch == CO_PATCH);

	return 0;
}
