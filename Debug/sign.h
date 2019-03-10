/*
 * Copyright (C) 2017 Nagravision S.A.
 */
#pragma once

int crypto_derive_keypair (const unsigned char *seed, unsigned char *pk, unsigned char *sk);

int crypto_sign (unsigned char *sm,
                 unsigned long long *smlen,
                 const unsigned char *m,
                 unsigned long long mlen,
                 const unsigned char *sk);

int crypto_sign_open (unsigned char *m,
                      unsigned long long *mlen,
                      const unsigned char *sm,
                      unsigned long long smlen,
                      const unsigned char *pk);
