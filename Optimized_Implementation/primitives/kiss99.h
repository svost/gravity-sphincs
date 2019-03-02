#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct kiss99_ctx kiss99_ctx;

struct kiss99_ctx {
    uint32_t z;
    uint32_t w;
    uint32_t jsr;
    uint32_t jcong;
};

void kiss99_srand (kiss99_ctx *ctx, const uint8_t *seed, size_t seedlen);
void kiss99_rand_buf (kiss99_ctx *ctx, uint8_t *buf, size_t buflen);
uint32_t kiss99_rand (kiss99_ctx *ctx);
