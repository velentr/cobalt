/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/random.h>

#include "random.h"

void co_rng_init(struct co_rng *rng)
{
	rng->lfsr = 0;
}

static int co_rng_seed(struct co_rng *rng)
{
	ssize_t size;
	char *seed;

	seed = getenv("COBALT_SEED");
	if (seed == NULL) {
		size = getrandom(&rng->lfsr, sizeof(rng->lfsr), 0);
		if (size < 0) {
			return errno;
		} else if (size != sizeof(rng->lfsr) || rng->lfsr == 0) {
			rng->lfsr = 0;
			return EAGAIN;
		}
	} else {
		rng->lfsr = strtoul(seed, NULL, 10);
	}

	return 0;
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

