/*
 * Copyright (C) 2017 Nagravision S.A.
 */

#include "gravity.h"
#include "randombytes.h"
#include "sign.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define SKLEN sizeof (struct gravity_sk)
#define PKLEN sizeof (struct gravity_pk)
#define SIGLEN sizeof (struct gravity_sign)
#define N 32
#define ROUNDS 3

#if PRINTKEYS
static char* ts_bin2hex(const unsigned char *old, const size_t oldlen)
{
    char *result = (char*) malloc(oldlen * 2 + 1);
    size_t i, j;
    int b = 0;

    for (i = j = 0; i < oldlen; i++) {
        b = old[i] >> 4;
        result[j++] = (char) (87 + b + (((b - 10) >> 31) & -39));
        b = old[i] & 0xf;
        result[j++] = (char) (87 + b + (((b - 10) >> 31) & -39));
    }
    result[j] = '\0';
    return result;

}
#endif

    uint8_t sk[SKLEN];
    uint8_t pk[PKLEN];

int main () {

    unsigned long long smlen;
    unsigned long long mlen = N;
    uint8_t m[N];
    uint8_t *sm = malloc (N + SIGLEN);
    struct timeval tm1, tm2;
    unsigned long long usecs;
    int ret = -1;
    int i;

    uint8_t secret_seed[32] = {
        0x6e, 0xe5, 0x0b, 0xba, 0x4a, 0xe2, 0x80, 0xe5, 0x00, 0xc4, 0x34, 0xb3, 0x32, 0xe4, 0x3b, 0xa5,
        0xdc, 0x6d, 0xfc, 0xb9, 0x34, 0xb6, 0x2a, 0x23, 0x6a, 0xee, 0x8b, 0x5c, 0x6e, 0x96, 0xc7, 0x4c,
    };

    uint8_t rng_seed[32] = {
        0xd7, 0xca, 0x2d, 0xb8, 0x20, 0x41, 0x0b, 0x25, 0x25, 0x22, 0xef, 0x2a, 0xd8, 0x62, 0x45, 0xd9,
        0xdc, 0x0b, 0x96, 0x7a, 0xc6, 0x8d, 0x6f, 0x08, 0x5c, 0x8e, 0x3c, 0x91, 0x2e, 0x28, 0xd7, 0x3f,
    };

    if (!sm) {
        fprintf (stderr, "error: sm malloc failed\n");
        ret = 1;
        goto label_exit_0;
    }

    printf ("k\t%d\n", PORS_k);
    printf ("h\t%d\n", MERKLE_h);
    printf ("d\t%d\n", GRAVITY_d);
    printf ("c\t%d\n", GRAVITY_c);
    printf ("sk len\t%d\n", (int)SKLEN);
    printf ("pk len\t%d\n", (int)PKLEN);
    printf ("sig len\t%d\n", (int)SIGLEN);

#define MEASURE(s)                                                                 \
    do {                                                                           \
        gettimeofday (&tm2, NULL);                                                 \
        usecs = 1000000 * (tm2.tv_sec - tm1.tv_sec) + (tm2.tv_usec - tm1.tv_usec); \
        printf ("\n# %s\n", s);                                                    \
        printf ("%.2f usec\n", (double)usecs);                                     \
    } while (0)

    struct csrng_ctx ctx;
    csrng_seed(&ctx, rng_seed, sizeof(rng_seed));
    csrng_bytes(&ctx, m, N);
    
#if PRINTKEYS
    uint8_t *rnd_hex = ts_bin2hex(m, N);
    printf("rnd_hex: %s\n", rnd_hex);
    free(rnd_hex);
#endif

    gettimeofday (&tm1, NULL);

    if (crypto_derive_keypair (secret_seed, pk, sk)) {
        fprintf (stderr, "error: crypto_derive_keypair failed\n");
        ret = 1;
        goto label_exit_2;
    }

    MEASURE ("crypto_derive_keypair");

#if PRINTKEYS
    uint8_t *pkh = ts_bin2hex(pk, PKLEN);
    uint8_t *skh = ts_bin2hex(sk, SKLEN);

    printf("Public key: %s\n", pkh);
    printf("Secret key: %s\n", skh);
    free(pkh);
    free(skh);
#endif

    for (i = 0; i < ROUNDS; ++i) {

        gettimeofday (&tm1, NULL);

        if (crypto_sign (sm, &smlen, m, mlen, sk)) {
            fprintf (stderr, "error: crypto_sign failed\n");
            ret = 1;
            goto label_exit_2;
        }

        MEASURE ("crypto_sign");
        gettimeofday (&tm1, NULL);


        if (crypto_sign_open (m, &mlen, sm, smlen, pk)) {
            fprintf (stderr, "error: crypto_sign_open failed\n");
            ret = 1;
            goto label_exit_2;
        }

        m[0] ^= sm[33];

        MEASURE ("crypto_sign_open");
    }

    ret = 0;
label_exit_2:
    free (sm);
label_exit_0:
    return ret;
}
