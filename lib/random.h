/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef RANDOM_H_
#define RANDOM_H_


#include <stdint.h>

struct co_rng {
	uint32_t lfsr;
};

/* initialize a random number generator */
void co_rng_init(struct co_rng *rng);

/* get a random object ID */
uint32_t co_getrandomid(struct co_rng *rng, int *err);


#endif /* end of include guard: RANDOM_H_ */

