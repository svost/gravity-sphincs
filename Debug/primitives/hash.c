/*
 * Copyright (C) 2017 Nagravision S.A.
 */
#include "../hash.h"
#include "haraka.h"
#include "../debug.h"

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
    PBYTES ("Data to hash", src, srclen);

    // Split original data to HASH_SIZE-byte chunks
    //    and use them to build a merkle tree.
    //
    // NOTE: We need a full set of HASH_SIZE-byte chunks. If case
    //   if we don't, then fill the remaining space with zeros.
    size_t full_chunks = srclen / HASH_SIZE; // Fully completed chunks
    size_t tail_len = srclen - HASH_SIZE * full_chunks; // Tail of data remaining after filling all of the completed chunks
    size_t total_chunks = (full_chunks == 0) ? 1 : (full_chunks + (tail_len != 0 ? 1 : 0)); // Full chunks plus uncompleted one if there is any tail present
    
    PINT("Full chunks", full_chunks);
    PINT("Tail length", tail_len);
    PINT("Total chunks", total_chunks);

    // Step 1: Populate zero level of nodes with chunks of input data
    struct hash * const nodes = malloc((total_chunks + 1) * sizeof(*nodes)); // +1 for duplication of odd element
    memset(&nodes[0], 0, (total_chunks + 1) * sizeof(*nodes));
    memcpy(&nodes[0], src, srclen); // copy src data
    
    PBYTES ("Nodes data array", &nodes[0], (total_chunks + 1) * sizeof(*nodes));

    // Step 2: Compress them to a merkle root hash
    //
    // NOTE: Please take into account that one-element node trees MUST be hashed anyway
    bool stop = false;
    for (size_t left = total_chunks; left > 0 && !stop; left /= 2)
    {
        if (left % 2 == 1)
        {
            // Duplicate last element if we have odd number of nodes
            memcpy(&nodes[left], &nodes[left - 1], sizeof(*nodes));
            PBYTES ("Duplicating odd element", &nodes[left - 1], sizeof(*nodes));
            PBYTES ("Updated nodes data array", &nodes[0], (total_chunks + 1) * sizeof(*nodes));
            if (left == 1) 
            {
                // We're at penultimate already, so 
                //   stop after this iteration will be done
                stop = true;
            }
            ++left;
        }

        for (size_t i = 0; i < left; i += 2)
        {
            // Turn a pair of nodes into upper node value
            haraka512_256(&nodes[i / 2].h[0], nodes[i].h);
            PINT("Pair #", i);
            PBYTES ("Updated nodes data array", &nodes[0], (total_chunks + 1) * sizeof(*nodes));
        }
    }

    memcpy(dst->h, &nodes[0].h[0], sizeof(*nodes));
    free(nodes);

    PBYTES ("Hashing result", dst->h, sizeof(*nodes));
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

