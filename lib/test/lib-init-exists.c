#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <cobalt/cobalt.h>

/* Check that co_init() properly fails if a file name .cobalt already exists */
int main()
{
	struct cobalt *co;
	int err = 0xdeadbeef;
	int fd;

	fd = open(".cobalt", O_CREAT | O_RDONLY, 0644);
	assert(fd != -1);

	co = co_init(".", &err);
	assert(co == NULL);
	assert(err == EEXIST);
	co_free(co);

	close(fd);

	return 0;
}
