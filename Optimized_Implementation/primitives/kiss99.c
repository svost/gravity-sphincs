#include "kiss99.h"

void kiss99_srand (kiss99_ctx *ctx, const uint8_t *seed, int seedlen) {
    int i;
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
