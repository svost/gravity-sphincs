/*
 *  Alexey Demidov
 *  Radius Group, LLC
 *  balthazar@yandex.ru
 *
 *  Microsoft Reference Source License (Ms-RSL)
 */

// g++ aesneon.cxx -mcpu=cortex-a53+simd+crypto -std=c++11

#include <stddef.h>
#include <arm_neon.h>

#include "../aes.h"

static int32x4_t assist256_1 (int32x4_t a, int32x4_t b) {
    int32x4_t c = {};
    b = vdupq_laneq_s32(b, 3); // shuffle ( , 0xff or 3,3,3,3)
    c = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(a), 12)); // slli (12 = 16 - 4)
    a = veorq_s32(a, c); // xor
    c = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(c), 12));
    a = veorq_s32(a, c);
    c = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(c), 12));
    a = veorq_s32(a, c);
    return veorq_s32(a, b); // return a = veorq_s32(a, b);
}

static int32x4_t assist256_2 (int32x4_t a, int32x4_t c) {
    int32x4_t b = {}, d = {};

    d = (int32x4_t)vaeseq_u8((uint8x16_t)a, (uint8x16_t){});
    uint8x16_t d_tmp {(uint8x16_t)d}; //d
    uint8x16_t dest = {
        d_tmp[0x4], d_tmp[0x1], d_tmp[0xE], d_tmp[0xB],
        d_tmp[0x1], d_tmp[0xE], d_tmp[0xB], d_tmp[0x4],
        d_tmp[0xC], d_tmp[0x9], d_tmp[0x6], d_tmp[0x3],
        d_tmp[0x9], d_tmp[0x6], d_tmp[0x3], d_tmp[0xC]
    };
    d = (int32x4_t)dest; //d = dest ^ (int32x4_t)((uint32x4_t){0, rcon, 0, rcon}); drop xor - rcon == 0
    b = vdupq_laneq_s32(d, 2); // shuffle ( , 0xaa or 2,2,2,2)
    d = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(c), 12));
    c = veorq_s32(c, d);
    d = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(d), 12));
    c = veorq_s32(c, d);
    d = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(d), 12));
    c = veorq_s32(c, d);
    return veorq_s32(c, b); // return c = veorq_s32(c, b);
}

static  int32x4_t aeskeygenassist (int32x4_t a, unsigned rcon) {

    a = (int32x4_t)vaeseq_u8((uint8x16_t)a, (uint8x16_t){});
    uint8x16_t d_tmp = {(uint8x16_t)a};
    uint8x16_t dest = {
        d_tmp[0x4], d_tmp[0x1], d_tmp[0xE], d_tmp[0xB],
        d_tmp[0x1], d_tmp[0xE], d_tmp[0xB], d_tmp[0x4],
        d_tmp[0xC], d_tmp[0x9], d_tmp[0x6], d_tmp[0x3],
        d_tmp[0x9], d_tmp[0x6], d_tmp[0x3], d_tmp[0xC]
    };
    return (int32x4_t)(vreinterpretq_u32_u8(dest) ^ (uint32x4_t){0, rcon, 0, rcon});
}

void aes256_KeyExpansion_NI(int32x4_t* keyExp, const int32x4_t* userkey)
{
    int32x4_t temp1, temp2, temp3;

    temp1 = keyExp[0] = vld1q_s32((int32_t *)userkey);
    temp3 = keyExp[1] = vld1q_s32((int32_t *)(userkey+1));

    temp2 = aeskeygenassist(temp3, 0x01);
    temp1 = keyExp[2] = assist256_1(temp1, temp2);
    temp3 = keyExp[3] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x02);
    temp1 = keyExp[4] = assist256_1(temp1, temp2);
    temp3 = keyExp[5] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x04);
    temp1 = keyExp[6] = assist256_1(temp1, temp2);
    temp3 = keyExp[7] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x08);
    temp1 = keyExp[8] = assist256_1(temp1, temp2);
    temp3 = keyExp[9] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x10);
    temp1 = keyExp[10] = assist256_1(temp1, temp2);
    temp3 = keyExp[11] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x20);
    temp1 = keyExp[12] = assist256_1(temp1, temp2);
    temp3 = keyExp[13] = assist256_2(temp1, temp3);

    keyExp[14] = assist256_1(temp1, aeskeygenassist(temp3, 0x40));
}

static int32x4_t increment_be_neon(int32x4_t x) {
    uint8x16_t swaporderq = {11, 10, 9, 8, 15, 14, 13, 12, 3, 2, 1, 0, 7, 6, 5, 4};
    x = vreinterpretq_s32_s8(vqtbl1q_s8(vreinterpretq_s8_s32(x), swaporderq));
    x = vaddq_s32(x, int32x4_t{0, 0x01, 0, 0});
    x = vreinterpretq_s32_s8(vqtbl1q_s8(vreinterpretq_s8_s32(x), swaporderq));
    return x;
}

void aesctr256_direct_x4 (uint8_t *out, const int32x4_t *rkeys, const void *counter, size_t bytes) {
    uint8x16_t s1, s2, s3, s4;
    int32x4_t ctr, *bo;
    /* bytes will always be a multiple of 16 */
    int blocks = bytes / 16;
    int blocks_parallel = 4 * (blocks / 4);
    int blocks_left = blocks - blocks_parallel;
    int i;

    ctr = vld1q_s32((int32_t *)counter);
    bo = (int32x4_t *)out;

    for (i = 0; i < blocks_parallel; i += 4) {
        s1 = vreinterpretq_u8_s32(veorq_s32(ctr, rkeys[0]));
        ctr = increment_be_neon(ctr);
        s2 = vreinterpretq_u8_s32(veorq_s32(ctr, rkeys[0]));
        ctr = increment_be_neon(ctr);
        s3 = vreinterpretq_u8_s32(veorq_s32(ctr, rkeys[0]));
        ctr = increment_be_neon(ctr);
        s4 = vreinterpretq_u8_s32(veorq_s32(ctr, rkeys[0]));
        ctr = increment_be_neon(ctr);

        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[1]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[1]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[1]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[1]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[2]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[2]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[2]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[2]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[3]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[3]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[3]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[3]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[4]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[4]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[4]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[4]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[5]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[5]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[5]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[5]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[6]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[6]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[6]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[6]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[7]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[7]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[7]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[7]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[8]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[8]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[8]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[8]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[9]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[9]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[9]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[9]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[10]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[10]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[10]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[10]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[11]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[11]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[11]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[11]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[12]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[12]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[12]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[12]);
        s1 = vaesmcq_u8(vaeseq_u8(s1, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[13]);
        s2 = vaesmcq_u8(vaeseq_u8(s2, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[13]);
        s3 = vaesmcq_u8(vaeseq_u8(s3, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[13]);
        s4 = vaesmcq_u8(vaeseq_u8(s4, (uint8x16_t){})) ^ vreinterpretq_u8_s32(rkeys[13]);
        s1 = vaeseq_u8(s1, (uint8x16_t){}) ^ vreinterpretq_u8_s32(rkeys[14]);
        s2 = vaeseq_u8(s2, (uint8x16_t){}) ^ vreinterpretq_u8_s32(rkeys[14]);
        s3 = vaeseq_u8(s3, (uint8x16_t){}) ^ vreinterpretq_u8_s32(rkeys[14]);
        s4 = vaeseq_u8(s4, (uint8x16_t){}) ^ vreinterpretq_u8_s32(rkeys[14]);

        vst1q_s32((int32_t*)(bo + i), vreinterpretq_s32_u8(s1));
        vst1q_s32((int32_t*)(bo + i + 1), vreinterpretq_s32_u8(s2));
        vst1q_s32((int32_t*)(bo + i + 2), vreinterpretq_s32_u8(s3));
        vst1q_s32((int32_t*)(bo + i + 3), vreinterpretq_s32_u8(s4));
    }

    for (i = 0; i < blocks_left; i++) {
        s1 = vreinterpretq_u8_s32(veorq_s32(ctr, rkeys[0]));
        ctr = increment_be_neon (ctr);
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[1]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[2]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[3]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[4]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[5]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[6]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[7]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[8]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[9]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[10]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[11]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[12]));
        s1 = vreinterpretq_u8_s32(veorq_s32(vreinterpretq_s32_u8(s1), rkeys[13]));
        s1 = vaeseq_u8(s1, (uint8x16_t){}) ^ vreinterpretq_u8_s32(rkeys[14]);

        vst1q_s32((int32_t*)(bo + blocks_parallel + i), vreinterpretq_s32_u8(s1));
    }
}

int aesctr256_zeroiv (uint8_t *out, const uint8_t *sk, int bytes) {
    uint8_t counter[16] = {0};
    return aesctr256(out, sk, counter, bytes);
}

int aesctr256 (uint8_t *out, const uint8_t *k, const void *counter, int bytes) {
    int32x4_t rkeys[15];
    expand256 (rkeys, (int32x4_t *)k);
    aesctr256_direct_x4 (out, rkeys, counter, bytes);
    return 0;
}
