#pragma once

#include <stdint.h>

typedef struct kiss99_ctx kiss99_ctx;

struct kiss99_ctx {
    uint32_t z;
    uint32_t w;
    uint32_t jsr;
    uint32_t jcong;
};

void kiss99_srand (kiss99_ctx *ctx, const uint8_t *seed, int seedlen);
uint32_t kiss99_rand (kiss99_ctx *tcx);
