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

void hash_to_N(struct hash *dst, const uint8_t *restrict src, uint64_t srclen)
{
    // Split original data to HASH_SIZE-byte chunks
    //    and use them to build a merkle tree.
    //
    // NOTE: We need a full set of HASH_SIZE-byte chunks. If case
    //   if we don't, then fill the remaining space with random data.
    size_t full_chunks = srclen / HASH_SIZE;
    size_t tail_len = srclen % HASH_SIZE;
    size_t total_chunks = full_chunks + !!tail_len;

    // Step 1: Populate zero level of nodes with chunks of input data
    uint8_t *restrict mem = malloc((total_chunks + 1) * HASH_SIZE); // +1 for duplication of odd element
    memcpy(mem, src, srclen); // copy src data

    // Step 2: Add pseudo-random padding to the end of data stream, if needed.
    if (tail_len) {
        kiss99_ctx ctx;
        kiss99_srand(&ctx, mem, srclen);
        kiss99_rand_buf(&ctx, mem + srclen, HASH_SIZE - tail_len);
    }

    // Step 3: If we have just one chunk, then create additional
    //   element to have even number of nodes
    if (total_chunks == 1)
    {
        haraka256_256(mem + HASH_SIZE, mem);
        ++total_chunks;
    }

    // Step 4: Compress all chunks to a merkle root hash
    //
    // NOTE: Please take into account that one-element node trees MUST be hashed anyway
    for (size_t left = total_chunks; left > 1; left /= 2)
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

