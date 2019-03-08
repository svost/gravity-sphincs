/*
 * Copyright (C) 2017 Nagravision S.A.
 */
#pragma once

#include "common.h"

#include <stdint.h>
#include <string.h>

/* TODO: portable alignment */
struct hash {
    uint8_t h[HASH_SIZE];
} __attribute__ ((aligned (16)));


void hash_2N_to_N (struct hash *dst, const struct hash *src);
void hash_to_N (struct hash *dst, const uint8_t *restrict src, uint64_t srclen);

/* int hashcmp(const struct hash *a, const struct hash *b); */
#define hashcmp(a, b) memcmp ((a)->h, (b)->h, HASH_SIZE)
/* int hashcmpN(const struct hash *a, const struct hash *b, size_t count); */
#define hashcmpN(a, b, N) memcmp ((a)->h, (b)->h, (N)*HASH_SIZE)
/* void hashcpy(struct hash *dst, const struct hash *src); */
#define hashcpy(a, b) memcpy ((a)->h, (b)->h, HASH_SIZE)
/* void hashcpyN(struct hash *dst, const struct hash *src, size_t count); */
#define hashcpyN(a, b, N) memcpy ((a)->h, (b)->h, (N)*HASH_SIZE)
/* void hashzero(struct hash *dst); */
#define hashzero(a) memset ((a)->h, 0, HASH_SIZE)
/* void swap(struct hash *a, struct hash *b, struct hash *tmp); */
#define hashswap(a, b, tmp)                                                    \
    do {                                                                       \
        (tmp) = (a);                                                           \
        (a) = (b);                                                             \
        (b) = (tmp);                                                           \
    } while (0);
