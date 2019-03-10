#pragma once

#include "primitives/kiss99.h"

struct csrng_ctx {
    uint8_t ctr[16];
    kiss99_ctx kiss99;
};

void csrng_seed(struct csrng_ctx *ctx, const uint8_t *seed, int seedlen);
void csrng_bytes(struct csrng_ctx *ctx, uint8_t *buf, int buflen);
