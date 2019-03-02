#include "kiss99.h"

// Seeding the KISS99 context by buffer data
void kiss99_srand (kiss99_ctx *ctx, const uint8_t *seed, size_t seedlen) {
    size_t i;
    ctx->z = 362436069;
    ctx->w = 521288629;
    ctx->jsr = 123456789;
    ctx->jcong = 380116160;

    for (i = 3; i < seedlen; i += 4) {
        ctx->z ^= seed[i - 3];
        ctx->w ^= seed[i - 2];
        ctx->jsr ^= seed[i - 1];
        ctx->jcong ^= seed[i];
        kiss99_rand (ctx);
    }

    if (i - 3 < seedlen) ctx->z ^= seed[i - 3];
    if (i - 2 < seedlen) ctx->w ^= seed[i - 2];
    if (i - 1 < seedlen) ctx->jsr ^= seed[i - 1];
}

// Use KISS algorithm to fill the given buffer with pseudo-random bytes
void kiss99_rand_buf (kiss99_ctx *ctx, uint8_t *buf, size_t buflen) {
    size_t d = buflen / 4;
    size_t m = buflen % 4;

    typedef union {
        uint8_t chX[4];
        uint32_t x;
    } item;

    item *items = (item *)buf;

    // Generate quotient number of unsigned 32-bit integers
    //   and convert them to bytes.
    for (size_t i = 0; i < d; ++i) {
        items[i].x = kiss99_rand(ctx);
    }

    // If there is a non-zero remainder then we need to get corresponding number
    //   of bytes from the last generated number
    if (m) {
        item tmp;
        tmp.x = kiss99_rand(ctx);
        for (size_t i = 0; i < m; ++i) {
            items[d].chX[i] = tmp.chX[i];
        }
    }
}

// https://en.wikipedia.org/wiki/KISS_(algorithm)
uint32_t kiss99_rand (kiss99_ctx *ctx) {
    ctx->z = 36969 * (ctx->z & 0xFFFF) + (ctx->z >> 16);
    ctx->w = 18000 * (ctx->w & 0xFFFF) + (ctx->w >> 16);
    uint32_t mwc = (ctx->z << 16) + ctx->w;
    ctx->jsr ^= (ctx->jsr << 17);
    ctx->jsr ^= (ctx->jsr >> 13);
    ctx->jsr ^= (ctx->jsr << 5);
    ctx->jcong = 69069 * ctx->jcong + 1234567;

    return (mwc ^ ctx->jcong) + ctx->jsr;
}
