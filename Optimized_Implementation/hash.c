/*
 * Copyright (C) 2017 Nagravision S.A.
 */
#include "hash.h"
#include "primitives/haraka.h"
#include "primitives/kiss99.h"

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

void hash_4N_to_4N(struct hash *dst, const struct hash *src)
{
    haraka256_256_4x(dst->h, src->h);
}

void hash_4N_to_4N_chain(struct hash *dst, const struct hash *src, int chainlen)
{
    haraka256_256_4x_chain(dst->h, src->h, chainlen);
}

void hash_8N_to_4N(struct hash *dst, const struct hash *src)
{
    haraka512_256_4x(dst->h, src->h);
}

void hash_to_N(struct hash *dst, const uint8_t *src, uint64_t srclen)
{
    // Split original data to HASH_SIZE-byte chunks
    //    and use them to build a merkle tree.
    //
    // NOTE: We need a full set of HASH_SIZE-byte chunks. If case
    //   if we don't, then fill the remaining space with zeros.
    size_t full_chunks = srclen / HASH_SIZE;
    size_t tail_len = srclen % HASH_SIZE;
    size_t total_chunks = full_chunks + !!tail_len;

    // Step 1: Populate zero level of nodes with chunks of input data
    struct hash * nodes = malloc((total_chunks + 1) * sizeof(*nodes)); // +1 for duplication of odd element
    memset((unsigned char*)&nodes[0], 0, (total_chunks + 1) * sizeof(*nodes));
    memcpy((unsigned char*)&nodes[0], src, srclen); // copy src data

    // Step 2: If we have just one chunk, then create additional
    //   element to have even number of nodes
    if (total_chunks == 1)
    {
        haraka256_256(nodes[1].h, nodes[0].h);
        ++total_chunks;
    }
    
    // Step 3: Compress all chunks to a merkle root hash
    //
    // NOTE: Please take into account that one-element node trees MUST be hashed anyway
    for (size_t left = total_chunks; left > 1; left /= 2)
    {
        if (left % 2 == 1)
        {
            // Append an image of last element if we have odd number of nodes
            haraka256_256(nodes[left].h, nodes[left - 1].h);
            ++left;
        }

        for (size_t i = 0; i < left; i += 2)
        {
            // Turn a pair of nodes into upper node value
            haraka512_256(nodes[i / 2].h, nodes[i].h);
        }
    }

    memcpy(dst->h, nodes[0].h, sizeof(*nodes));
    free(nodes);
}

void hash_compress_pairs(struct hash *dst, const struct hash *src, int count)
{
    int i = 0;
    for (; i+4 <= count; i+=4)
        hash_8N_to_4N(&dst[i], &src[2*i]);
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
    for (; i+4 <= count; i+=4)
        hash_4N_to_4N(&dst[i], &src[i]);
    for (; i < count; ++i)
        hash_N_to_N(&dst[i], &src[i]);
}

void hash_parallel_chains(struct hash *dst, const struct hash *src, int count, int chainlen)
{
    int i = 0;
    for (; i+4 <= count; i+=4)
        hash_4N_to_4N_chain(&dst[i], &src[i], chainlen);
    for (; i < count; ++i)
        hash_N_to_N_chain(&dst[i], &src[i], chainlen);
}

