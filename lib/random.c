/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "random.h"

void co_rng_init(struct co_rng *rng)
{
	rng->lfsr = 0;
}

static ssize_t co_getrandom(void *buf, size_t buflen)
{
	ssize_t size;
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return -1;

	size = read(fd, buf, buflen);
	close(fd);

	return size;
}

static int co_rng_seed(struct co_rng *rng)
{
	ssize_t size;
	char *seed;

	seed = getenv("COBALT_SEED");
	if (seed == NULL) {
		size = co_getrandom(&rng->lfsr, sizeof(rng->lfsr));
		if (size < 0)
			return errno;
		else if (size != sizeof(rng->lfsr) || rng->lfsr == 0)
			rng->lfsr = 0;
	} else {
		rng->lfsr = strtoul(seed, NULL, 10);
	}

	return rng->lfsr == 0 ? EAGAIN : 0;
}

uint32_t co_getrandomid(struct co_rng *rng, int *err)
{
	int bit;

	if (rng->lfsr == 0) {
		*err = co_rng_seed(rng);
		if (rng->lfsr == 0)
			return 0;
	} else {
		bit = __builtin_parity(rng->lfsr & 0x80000057u);
		rng->lfsr = (rng->lfsr << 1) | bit;
	}

	return rng->lfsr;
}

