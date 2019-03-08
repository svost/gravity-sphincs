/*
 * Copyright (C) 2017 Nagravision S.A.
 */
#include "../hash.h"
#include "haraka.h"
#include "kiss99.h"

#include <stdlib.h>
#include <stdbool.h>

void hash_N_to_N(struct hash *dst, const struct hash *src)
{
    haraka256_256(dst->h, src->h);
}

void hash_N_to_N_chain(struct hash *dst, const struct hash *src, int chainlen)
{
    haraka256_256_chain(dst->h, src->h, chainlen);
}

void hash_2N_to_N(struct hash *dst, const struct hash *src)
{
    haraka512_256(dst->h, src->h);
}

void hash_to_N(struct hash *dst, const uint8_t *restrict src, uint32_t srclen)
{
    uint32_t nPaddSz = 2 * HASH_SIZE;

    // Split original data to 2 * HASH_SIZE-byte chunks
    //    and use them to build a merkle tree.
    size_t full_chunks = srclen / nPaddSz;
    size_t tail_len = srclen % nPaddSz;
    size_t total_chunks = full_chunks + !!tail_len;

    // Step 1: Populate zero level of nodes with chunks of input data
    uint8_t *restrict mem = malloc((total_chunks + 1) * nPaddSz); // +2 for worst case of padding
    memcpy(mem, src, srclen); // copy src data

    // Step 2: To prevent collision attacks, we always need to append a data set to have 2n * HASH_SIZE bytes.
    kiss99_ctx ctx;
    uint32_t nsf = srclen;
    uint8_t *restrict seed = malloc(tail_len + 4);
    seed[0] = (uint8_t)  nsf;
    seed[1] = (uint8_t) (nsf >>= 8);
    seed[2] = (uint8_t) (nsf >>= 8);
    seed[3] = (uint8_t) (nsf >>= 8);
    memcpy(seed + 4, mem + srclen - tail_len, tail_len);
    kiss99_srand(&ctx, seed, tail_len + 4);
    free(seed);

    uint32_t padd_size = nPaddSz - tail_len;
    kiss99_rand_buf(&ctx, mem + srclen, padd_size);

    if (padd_size == 64) {
        total_chunks++;
    }

    // Step 3: Compress all chunks to a merkle root hash
    for (size_t left = 2 * total_chunks; left > 1; left /= 2)
    {
        if (left % 2 == 1)
        {
            // Append an image of last element if we have odd number of nodes
            haraka256_256(mem + left * HASH_SIZE, mem + (left - 1) * HASH_SIZE);
            ++left;
        }

        for (size_t i = 0; i < left; i += 2)
        {
            // Turn a pair of nodes into upper node value
            haraka512_256(mem + i * HASH_SIZE / 2, mem + i * HASH_SIZE);
        }
    }

    memcpy(&dst->h[0], mem, HASH_SIZE);
    free(mem);
}

void hash_compress_pairs(struct hash *dst, const struct hash *src, int count)
{
    int i = 0;
    for (; i < count; ++i)
        hash_2N_to_N(&dst[i], &src[2*i]);
}

void hash_compress_all(struct hash *dst, const struct hash *src, int count)
{
    hash_to_N(dst, src->h, count * HASH_SIZE);
}

void hash_parallel(struct hash *dst, const struct hash *src, int count)
{
    int i = 0;
    for (; i < count; ++i)
        hash_N_to_N(&dst[i], &src[i]);
}

void hash_parallel_chains(struct hash *dst, const struct hash *src, int count, int chainlen)
{
    int i = 0;
    for (; i < count; ++i)
        hash_N_to_N_chain(&dst[i], &src[i], chainlen);
}

