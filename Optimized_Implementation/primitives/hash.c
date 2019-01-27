/*
 * Copyright (C) 2017 Nagravision S.A.
 */
#include "../hash.h"
#include "haraka.h"

#include <stdlib.h>

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
    // We should have a set of HASH_SIZE-byte chunks. In case if we don't, we have to append it.
    size_t full_chunks = srclen / HASH_SIZE;
    size_t excessive_data_len = srclen - HASH_SIZE * full_chunks;
    size_t total_chunks = full_chunks + (excessive_data_len != 0 ? 1 : 0);
    
    // Step 1: Populate zero level of tree
    struct hash * const nodes = malloc((total_chunks + 1) * sizeof(*nodes)); // +1 for duplication of odd element
    memset(&nodes[0], 0, (total_chunks + 1) * sizeof(*nodes));
    memcpy(&nodes[0], src, srclen); // src data + zero padding
    
    // Step 2: Compress them to a merkle root hash
    for ( ; total_chunks > 1; total_chunks /= 2)
    {
        // Duplicate last element if we have odd number of nodes
        if (total_chunks % 2 == 1)
        {
            memcpy(&nodes[total_chunks], &nodes[total_chunks - 1], sizeof(*nodes));
            ++total_chunks;
        }

        for (size_t i = 0; i < total_chunks; i += 2)
            haraka512_256(nodes[i / 2].h, nodes[i].h);
    }

    memcpy(dst->h, nodes, HASH_SIZE);
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

