#include "hash.h"
#include "randombytes.h"
#include "primitives/aes.h"

void csrng_seed(struct csrng_ctx *ctx, const uint8_t *seed, int seedlen) {
    struct hash seed_hash;
    hash_to_N (&seed_hash, seed, seedlen);
    memcpy(ctx->ctr, seed_hash.h, 16);
    kiss99_srand (&ctx->kiss99, seed_hash.h, HASH_SIZE);
}

void csrng_bytes(struct csrng_ctx *ctx, uint8_t *buf, int buflen) {
    uint8_t key[32];
    kiss99_rand_buf (&ctx->kiss99, key, 32);
    kiss99_rand_buf (&ctx->kiss99, buf, buflen);
    aesctr256(buf, key, ctx->ctr, buflen);
}
