#pragma once

#include <stdint.h>

void haraka256_256(unsigned char *out, const unsigned char *in);
void haraka256_256_4x(unsigned char *out, const unsigned char *in);
void haraka256_256_chain(unsigned char *out, const unsigned char *in, int chainlen);
void haraka256_256_4x_chain(unsigned char *out, const unsigned char *in, int chainlen);
void haraka512_256(unsigned char *out, const unsigned char *in);
void haraka512_256_4x(unsigned char *out, const unsigned char *in);
