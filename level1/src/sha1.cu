#include <stdint.h>

#include "sha1.h"

__constant__ uint32_t c_block[16];
__constant__ hash_digest_t c_ctx;
__constant__ uint32_t c_mask;

__device__ __forceinline__ uint32_t f1(uint32_t b, uint32_t c, uint32_t d){
    return (b & c) | ((~b) & d);
}

__device__ __forceinline__ uint32_t computeSHA1Block(uint32_t* in, uint32_t id, uint32_t idx,
                                                     hash_digest_t* h)
{
    uint32_t a = h->h0;
    uint32_t b = h->h1;
    uint32_t c = h->h2;
    uint32_t d = h->h3;
    uint32_t e = h->h4;
    uint32_t f;
    uint32_t k;
    uint32_t temp;
    uint32_t w[16];
    int i;

#pragma unroll 11
    for (i = 0; i < 11; i++) {
        w[i] = in[i];
    }

    w[11] = idx;
    w[12] = id;

#pragma unroll 3
    for (i = 13; i < 16; i++) {
        w[i] = in[i];
    }

    k = 0x5A827999;
    //0 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[0];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[0] = w[13] ^ w[8] ^ w[2] ^ w[0];
    w[0] = w[0] << 1 | w[0] >> 31;

    //1 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[1];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[1] = w[14] ^ w[9] ^ w[3] ^ w[1];
    w[1] = w[1] << 1 | w[1] >> 31;

    //2 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[2];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[2] = w[15] ^ w[10] ^ w[4] ^ w[2];
    w[2] = w[2] << 1 | w[2] >> 31;

    //3 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[3];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[3] = w[0] ^ w[11] ^ w[5] ^ w[3];
    w[3] = w[3] << 1 | w[3] >> 31;

    //4 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[4];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[4] = w[1] ^ w[12] ^ w[6] ^ w[4];
    w[4] = w[4] << 1 | w[4] >> 31;

    //5 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[5];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[5] = w[2] ^ w[13] ^ w[7] ^ w[5];
    w[5] = w[5] << 1 | w[5] >> 31;

    //6 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[6];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[6] = w[3] ^ w[14] ^ w[8] ^ w[6];
    w[6] = w[6] << 1 | w[6] >> 31;

    //7 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[7];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[7] = w[4] ^ w[15] ^ w[9] ^ w[7];
    w[7] = w[7] << 1 | w[7] >> 31;

    //8 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[8];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[8] = w[5] ^ w[0] ^ w[10] ^ w[8];
    w[8] = w[8] << 1 | w[8] >> 31;

    //9 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[9];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[9] = w[6] ^ w[1] ^ w[11] ^ w[9];
    w[9] = w[9] << 1 | w[9] >> 31;

    //10 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[10];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[10] = w[7] ^ w[2] ^ w[12] ^ w[10];
    w[10] = w[10] << 1 | w[10] >> 31;

    //11 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[11];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[11] = w[8] ^ w[3] ^ w[13] ^ w[11];
    w[11] = w[11] << 1 | w[11] >> 31;

    //12 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[12];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[12] = w[9] ^ w[4] ^ w[14] ^ w[12];
    w[12] = w[12] << 1 | w[12] >> 31;

    //13 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[13];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[13] = w[10] ^ w[5] ^ w[15] ^ w[13];
    w[13] = w[13] << 1 | w[13] >> 31;

    //14 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[14];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[14] = w[11] ^ w[6] ^ w[0] ^ w[14];
    w[14] = w[14] << 1 | w[14] >> 31;

    //15 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[15];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[15] = w[12] ^ w[7] ^ w[1] ^ w[15];
    w[15] = w[15] << 1 | w[15] >> 31;

    //16 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[0];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[0] = w[13] ^ w[8] ^ w[2] ^ w[0];
    w[0] = w[0] << 1 | w[0] >> 31;

    //17 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[1];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[1] = w[14] ^ w[9] ^ w[3] ^ w[1];
    w[1] = w[1] << 1 | w[1] >> 31;

    //18 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[2];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[2] = w[15] ^ w[10] ^ w[4] ^ w[2];
    w[2] = w[2] << 1 | w[2] >> 31;

    //19 of 0-20
    temp = ((a << 5) | (a >> 27)) + f1(b, c, d) + e + k + w[3];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[3] = w[0] ^ w[11] ^ w[5] ^ w[3];
    w[3] = w[3] << 1 | w[3] >> 31;

    k = 0x6ED9EBA1;
    //20 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[4];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[4] = w[1] ^ w[12] ^ w[6] ^ w[4];
    w[4] = w[4] << 1 | w[4] >> 31;

    //21 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[5];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[5] = w[2] ^ w[13] ^ w[7] ^ w[5];
    w[5] = w[5] << 1 | w[5] >> 31;

    //22 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[6];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[6] = w[3] ^ w[14] ^ w[8] ^ w[6];
    w[6] = w[6] << 1 | w[6] >> 31;

    //23 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[7];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[7] = w[4] ^ w[15] ^ w[9] ^ w[7];
    w[7] = w[7] << 1 | w[7] >> 31;

    //24 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[8];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[8] = w[5] ^ w[0] ^ w[10] ^ w[8];
    w[8] = w[8] << 1 | w[8] >> 31;

    //25 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[9];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[9] = w[6] ^ w[1] ^ w[11] ^ w[9];
    w[9] = w[9] << 1 | w[9] >> 31;

    //26 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[10];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[10] = w[7] ^ w[2] ^ w[12] ^ w[10];
    w[10] = w[10] << 1 | w[10] >> 31;

    //27 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[11];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[11] = w[8] ^ w[3] ^ w[13] ^ w[11];
    w[11] = w[11] << 1 | w[11] >> 31;

    //28 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[12];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[12] = w[9] ^ w[4] ^ w[14] ^ w[12];
    w[12] = w[12] << 1 | w[12] >> 31;

    //29 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[13];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[13] = w[10] ^ w[5] ^ w[15] ^ w[13];
    w[13] = w[13] << 1 | w[13] >> 31;

    //30 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[14];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[14] = w[11] ^ w[6] ^ w[0] ^ w[14];
    w[14] = w[14] << 1 | w[14] >> 31;

    //31 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[15];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[15] = w[12] ^ w[7] ^ w[1] ^ w[15];
    w[15] = w[15] << 1 | w[15] >> 31;

    //32 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[0];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[0] = w[13] ^ w[8] ^ w[2] ^ w[0];
    w[0] = w[0] << 1 | w[0] >> 31;

    //33 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[1];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[1] = w[14] ^ w[9] ^ w[3] ^ w[1];
    w[1] = w[1] << 1 | w[1] >> 31;

    //34 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[2];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[2] = w[15] ^ w[10] ^ w[4] ^ w[2];
    w[2] = w[2] << 1 | w[2] >> 31;

    //35 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[3];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[3] = w[0] ^ w[11] ^ w[5] ^ w[3];
    w[3] = w[3] << 1 | w[3] >> 31;

    //36 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[4];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[4] = w[1] ^ w[12] ^ w[6] ^ w[4];
    w[4] = w[4] << 1 | w[4] >> 31;

    //37 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[5];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[5] = w[2] ^ w[13] ^ w[7] ^ w[5];
    w[5] = w[5] << 1 | w[5] >> 31;

    //38 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[6];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[6] = w[3] ^ w[14] ^ w[8] ^ w[6];
    w[6] = w[6] << 1 | w[6] >> 31;

    //39 of 20-40
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[7];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[7] = w[4] ^ w[15] ^ w[9] ^ w[7];
    w[7] = w[7] << 1 | w[7] >> 31;

    k = 0x8F1BBCDC;
    //40 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[8];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[8] = w[5] ^ w[0] ^ w[10] ^ w[8];
    w[8] = w[8] << 1 | w[8] >> 31;

    //41 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[9];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[9] = w[6] ^ w[1] ^ w[11] ^ w[9];
    w[9] = w[9] << 1 | w[9] >> 31;

    //42 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[10];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[10] = w[7] ^ w[2] ^ w[12] ^ w[10];
    w[10] = w[10] << 1 | w[10] >> 31;

    //43 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[11];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[11] = w[8] ^ w[3] ^ w[13] ^ w[11];
    w[11] = w[11] << 1 | w[11] >> 31;

    //44 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[12];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[12] = w[9] ^ w[4] ^ w[14] ^ w[12];
    w[12] = w[12] << 1 | w[12] >> 31;

    //45 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[13];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[13] = w[10] ^ w[5] ^ w[15] ^ w[13];
    w[13] = w[13] << 1 | w[13] >> 31;

    //46 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[14];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[14] = w[11] ^ w[6] ^ w[0] ^ w[14];
    w[14] = w[14] << 1 | w[14] >> 31;

    //47 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[15];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[15] = w[12] ^ w[7] ^ w[1] ^ w[15];
    w[15] = w[15] << 1 | w[15] >> 31;

    //48 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[0];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[0] = w[13] ^ w[8] ^ w[2] ^ w[0];
    w[0] = w[0] << 1 | w[0] >> 31;

    //49 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[1];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[1] = w[14] ^ w[9] ^ w[3] ^ w[1];
    w[1] = w[1] << 1 | w[1] >> 31;

    //50 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[2];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[2] = w[15] ^ w[10] ^ w[4] ^ w[2];
    w[2] = w[2] << 1 | w[2] >> 31;

    //51 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[3];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[3] = w[0] ^ w[11] ^ w[5] ^ w[3];
    w[3] = w[3] << 1 | w[3] >> 31;

    //52 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[4];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[4] = w[1] ^ w[12] ^ w[6] ^ w[4];
    w[4] = w[4] << 1 | w[4] >> 31;

    //53 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[5];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[5] = w[2] ^ w[13] ^ w[7] ^ w[5];
    w[5] = w[5] << 1 | w[5] >> 31;

    //54 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[6];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[6] = w[3] ^ w[14] ^ w[8] ^ w[6];
    w[6] = w[6] << 1 | w[6] >> 31;

    //55 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[7];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[7] = w[4] ^ w[15] ^ w[9] ^ w[7];
    w[7] = w[7] << 1 | w[7] >> 31;

    //56 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[8];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[8] = w[5] ^ w[0] ^ w[10] ^ w[8];
    w[8] = w[8] << 1 | w[8] >> 31;

    //57 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[9];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[9] = w[6] ^ w[1] ^ w[11] ^ w[9];
    w[9] = w[9] << 1 | w[9] >> 31;

    //58 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[10];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[10] = w[7] ^ w[2] ^ w[12] ^ w[10];
    w[10] = w[10] << 1 | w[10] >> 31;

    //59 of 40-60
    f = (b & c) | (b & d) | (c & d);
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[11];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[11] = w[8] ^ w[3] ^ w[13] ^ w[11];
    w[11] = w[11] << 1 | w[11] >> 31;

    k = 0xCA62C1D6;

    //60 of 60-64
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[12];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[12] = w[9] ^ w[4] ^ w[14] ^ w[12];
    w[12] = w[12] << 1 | w[12] >> 31;

    //61 of 60-64
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[13];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[13] = w[10] ^ w[5] ^ w[15] ^ w[13];
    w[13] = w[13] << 1 | w[13] >> 31;

    //62 of 60-64
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[14];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[14] = w[11] ^ w[6] ^ w[0] ^ w[14];
    w[14] = w[14] << 1 | w[14] >> 31;

    //63 of 60-64
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[15];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    w[15] = w[12] ^ w[7] ^ w[1] ^ w[15];
    w[15] = w[15] << 1 | w[15] >> 31;


    //64 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[0];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //65 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[1];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //66 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[2];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //67 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[3];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //68 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[4];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //69 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[5];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //70 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[6];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //71 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[7];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //72 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[8];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //73 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[9];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //74 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[10];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //75 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[11];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //76 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[12];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //77 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[13];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //78 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[14];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    //79 of 64-80
    f = b ^ c ^ d;
    temp = ((a << 5) | (a >> 27)) + f + e + k + w[15];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;

    return h->h0 + a;
}

__global__ void
__launch_bounds__(THREADS_PER_BLOCK) // Add BLOCKS_PER_SM arg to limit register usage
shaforce(volatile uint32_t* result,
                         const __restrict__ uint32_t idx)
{
    uint8_t i;
    uint32_t res;
    uint32_t global_id = blockIdx.x * blockDim.x + threadIdx.x;

    for(i = 0; i < 16; i++){
        global_id |= (i << 24);

        res = computeSHA1Block(c_block, global_id, idx, &c_ctx);

        if(!(res & c_mask)){
            // Add one so zero can signal not-found
            atomicMax((uint32_t*)result, global_id+1);
            break;
        }/* else if(*result){
            break;
            }*/

        global_id &= 0x00ffffff;
    }
}


extern "C" void force_kernel(unsigned int *d_result,
                             const uint32_t idx){
    shaforce<<<BLOCKS_PER_GRID, THREADS_PER_BLOCK>>>(d_result, idx);
}

extern "C" cudaError_t copy_constants(uint32_t *h_block,
                                      uint32_t *h_mask,
                                      hash_digest_t *h_ctx){
    return (cudaError_t)(
        cudaMemcpyToSymbol(c_block, h_block, sizeof(uint32_t) * 16) |
        cudaMemcpyToSymbol(c_mask, h_mask, sizeof(uint32_t)) |
        cudaMemcpyToSymbol(c_ctx, h_ctx, sizeof(hash_digest_t)));
}
