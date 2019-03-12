//  Created by Bassham, Lawrence E (Fed) on 8/29/17.
//  Copyright © 2017 Bassham, Lawrence E (Fed). All rights reserved.
//

#pragma once

#include <stdio.h>
#include <stdint.h>

#define RNG_SUCCESS      0
#define RNG_BAD_MAXLEN  -1
#define RNG_BAD_OUTBUF  -2
#define RNG_BAD_REQ_LEN -3

struct csrng_ctx {
    unsigned char   Key[32];
    unsigned char   V[16];
    unsigned int    reseed_counter;
} __attribute__ ((aligned (16)));


void AES256_CTR_DRBG_Update(uint8_t *provided_data, uint8_t *Key, uint8_t *V);

void csrng_seed(struct csrng_ctx *ctx, const uint8_t *entropy_input, const uint8_t *personalization_string);
int csrng_bytes(struct csrng_ctx *ctx, uint8_t *x, size_t xlen);
