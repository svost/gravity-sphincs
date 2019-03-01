#pragma once

#include <stdint.h>

#define LOAD(src) vld1q_u8((const uint8_t *)(src))

#define AES2(s0, s1, rci) \
  s0 = vaesmcq_u8(vaeseq_u8(s0, zero8x16)); \
  s1 = vaesmcq_u8(vaeseq_u8(s1, zero8x16)); \
  s0 = vaesmcq_u8(vaeseq_u8(s0, LOAD(&rc8x16[4*rci + 0]))) ^ LOAD(&rc8x16[4*rci + 2]); \
  s1 = vaesmcq_u8(vaeseq_u8(s1, LOAD(&rc8x16[4*rci + 1]))) ^ LOAD(&rc8x16[4*rci + 3]);

#define AES2_4x(s0, s1, s2, s3, rci) \
  AES2(s0[0], s0[1], rci); \
  AES2(s1[0], s1[1], rci); \
  AES2(s2[0], s2[1], rci); \
  AES2(s3[0], s3[1], rci);

#define MIX2(s0, s1) \
  tmp = (uint8x16_t) vzip2q_u32((uint32x4_t)s0, (uint32x4_t)s1); \
  s0 = (uint8x16_t) vzip1q_u32((uint32x4_t)s0, (uint32x4_t)s1); \
  s1 = tmp;

#define MIX2_4x(s0, s1, s2, s3) \
  MIX2(s0[0], s0[1]); \
  MIX2(s1[0], s1[1]); \
  MIX2(s2[0], s2[1]); \
  MIX2(s3[0], s3[1]);

#define AES_MIX2(s0, s1, rci) \
  AES2(s0, s1, rci); \
  MIX2(s0, s1);

#define MIX4(s0, s1, s2, s3) \
  tmp = vzip1q_u32((uint32x4_t)s0, (uint32x4_t)s1); \
  s0 = vreinterpretq_u8_u32(vzip2q_u32((uint32x4_t)s0, (uint32x4_t)s1)); \
  s1 = vreinterpretq_u8_u32(vzip1q_u32((uint32x4_t)s2, (uint32x4_t)s3)); \
  s2 = vreinterpretq_u8_u32(vzip2q_u32((uint32x4_t)s2, (uint32x4_t)s3)); \
  s3 = vreinterpretq_u8_u32(vzip1q_u32((uint32x4_t)s0, (uint32x4_t)s2)); \
  s0 = vreinterpretq_u8_u32(vzip2q_u32((uint32x4_t)s0, (uint32x4_t)s2)); \
  s2 = vreinterpretq_u8_u32(vzip2q_u32((uint32x4_t)s1, tmp)); \
  s1 = vreinterpretq_u8_u32(vzip1q_u32((uint32x4_t)s1, tmp));

#define AES4(s0, s1, s2, s3, rci ) \
  s0 = vaesmcq_u8(vaeseq_u8(s0, zero8x16)); \
  s1 = vaesmcq_u8(vaeseq_u8(s1, zero8x16)); \
  s0 = vaesmcq_u8(vaeseq_u8(s0, rc8x16[8*rci + 0])) ^ rc8x16[8*rci + 4]; \
  s1 = vaesmcq_u8(vaeseq_u8(s1, rc8x16[8*rci + 1])) ^ rc8x16[8*rci + 5]; \
  s2 = vaesmcq_u8(vaeseq_u8(s2, zero8x16)); \
  s3 = vaesmcq_u8(vaeseq_u8(s3, zero8x16)); \
  s2 = vaesmcq_u8(vaeseq_u8(s2, rc8x16[8*rci + 2])) ^ rc8x16[8*rci + 6]; \
  s3 = vaesmcq_u8(vaeseq_u8(s3, rc8x16[8*rci + 3])) ^ rc8x16[8*rci + 7];

void haraka256_256(unsigned char *out, const unsigned char *in);
void haraka256_256_4x(unsigned char *out, const unsigned char *in);
void haraka256_256_chain(unsigned char *out, const unsigned char *in, int chainlen);
void haraka256_256_4x_chain(unsigned char *out, const unsigned char *in, int chainlen);
void haraka512_256(unsigned char *out, const unsigned char *in);
void haraka512_256_4x(unsigned char *out, const unsigned char *in);
