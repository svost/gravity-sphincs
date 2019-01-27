/*
 * Copyright (C) 2017 Nagravision S.A.
 */
#include "../hash.h"
#include "haraka.h"
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

void hash_to_N(struct hash *dst, const uint8_t *src, uint64_t srclen)
{
    // Split original data to HASH_SIZE-byte chunks
    //    and use them to build a merkle tree.
    //
    // NOTE: We need a full set of HASH_SIZE-byte chunks. If case
    //   if we don't, then fill the remaining space with zeros.
    size_t full_chunks = srclen / HASH_SIZE; // Fully completed chunks
    size_t tail_len = srclen - HASH_SIZE * full_chunks; // Tail of data remaining after filling all of the completed chunks
    size_t total_chunks = (full_chunks == 0) ? 1 : (full_chunks + (tail_len != 0 ? 1 : 0)); // Full chunks plus uncompleted one if there is any tail present
    
    // Step 1: Populate zero level of nodes with chunks of input data
    struct hash * nodes = malloc((total_chunks + 1) * sizeof(*nodes)); // +1 for duplication of odd element
    memset(&nodes[0], 0, (total_chunks + 1) * sizeof(*nodes));
    memcpy(&nodes[0], src, srclen); // copy src data

    // Step 2: If we have just one chunk, then duplicate it
    if (total_chunks == 1)
    {
        memcpy(&nodes[1], src, srclen); // copy src data
        ++total_chunks;
    }

    // Step 3: Compress all chunks to a merkle root hash
    //
    // NOTE: Please take into account that one-element node trees MUST be hashed anyway
    for (size_t left = total_chunks; left > 1; left /= 2)
    {
        if (left % 2 == 1)
        {
            // Duplicate last element if we have odd number of nodes
            memcpy(&nodes[left], &nodes[left - 1], sizeof(*nodes));
            ++left;
        }

        for (size_t i = 0; i < left; i += 2)
        {
            // Turn a pair of nodes into upper node value
            haraka512_256(&nodes[i / 2].h[0], nodes[i].h);
        }
    }

    memcpy(dst->h, &nodes[0].h[0], sizeof(*nodes));
    free(nodes);
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

