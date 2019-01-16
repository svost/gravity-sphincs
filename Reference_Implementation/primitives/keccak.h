#pragma once
#include <stdint.h>

void Keccak(uint32_t rate, uint32_t capacity, const uint8_t *input, uint64_t inputByteLen, uint8_t delimitedSuffix, uint8_t *output, uint64_t outputByteLen);
void FIPS202_SHAKE128(const uint8_t *input, uint32_t inputByteLen, uint8_t *output, int outputByteLen);
void FIPS202_SHAKE256(const uint8_t *input, uint32_t inputByteLen, uint8_t *output, int outputByteLen);
void FIPS202_SHA3_224(const uint8_t *input, uint32_t inputByteLen, uint8_t *output);
void FIPS202_SHA3_256(const uint8_t *input, uint32_t inputByteLen, uint8_t *output);
void FIPS202_SHA3_384(const uint8_t *input, uint32_t inputByteLen, uint8_t *output);
void FIPS202_SHA3_512(const uint8_t *input, uint32_t inputByteLen, uint8_t *output);
