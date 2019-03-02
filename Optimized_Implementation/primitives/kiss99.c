#include "kiss99.h"

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

void kiss99_rand_buf (kiss99_ctx *ctx, uint8_t *buf, size_t buflen) {
    size_t d = buflen / 4;
    size_t m = buflen % 4;

    typedef union {
        uint8_t chX[4];
        uint32_t x;
    } item;

    item *items = (item *)buf;
    item tmp;

    for (size_t i = 0; i < d; ++i) {
        uint32_t znew = 36969 * (ctx->z & 0xFFFF) + (ctx->z >> 16);
        uint32_t wnew = 18000 * (ctx->w & 0xFFFF) + (ctx->w >> 16);
        uint32_t mwc = (znew << 16) + wnew;
        uint32_t shr3 = ctx->jsr ^ (ctx->jsr << 17);
        shr3 ^= shr3 >> 13;
        shr3 ^= shr3 << 5;
        uint32_t cong = 69069 * ctx->jcong + 1234567;

        ctx->z = znew;
        ctx->w = wnew;
        ctx->jsr = shr3;
        ctx->jcong = cong;

        tmp.x = (mwc ^ cong) + shr3;
        items[i] = tmp;
    }

    for (size_t i = 0; i < m; ++i) {
      items[d].chX[i] = tmp.chX[i];
    }
}

uint32_t kiss99_rand (kiss99_ctx *ctx) {
    uint32_t znew;
    uint32_t wnew;
    uint32_t mwc;
    uint32_t shr3;
    uint32_t cong;

    znew = 36969 * (ctx->z & 0xFFFF) + (ctx->z >> 16);
    wnew = 18000 * (ctx->w & 0xFFFF) + (ctx->w >> 16);
    mwc = (znew << 16) + wnew;
    shr3 = ctx->jsr ^ (ctx->jsr << 17);
    shr3 ^= shr3 >> 13;
    shr3 ^= shr3 << 5;
    cong = 69069 * ctx->jcong + 1234567;

    ctx->z = znew;
    ctx->w = wnew;
    ctx->jsr = shr3;
    ctx->jcong = cong;

    return (mwc ^ cong) + shr3;
}
