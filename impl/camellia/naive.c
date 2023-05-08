#include "naive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint64_t keysched_const[] = {
        0xa09e667f3bcc908bUL,
        0xb67ae8584caa73b2UL,
        0xc6ef372fe94f82beUL,
        0x54ff53a5f1d36f1cUL,
        0x10e527fade682d1dUL,
        0xb05688c2b3e6c1fdUL
};

// S-box 1
static const uint8_t s1[256] =
{
        0x70, 0x82, 0x2c, 0xec, 0xb3, 0x27, 0xc0, 0xe5,
        0xe4, 0x85, 0x57, 0x35, 0xea, 0x0c, 0xae, 0x41,
        0x23, 0xef, 0x6b, 0x93, 0x45, 0x19, 0xa5, 0x21,
        0xed, 0x0e, 0x4f, 0x4e, 0x1d, 0x65, 0x92, 0xbd,
        0x86, 0xb8, 0xaf, 0x8f, 0x7c, 0xeb, 0x1f, 0xce,
        0x3e, 0x30, 0xdc, 0x5f, 0x5e, 0xc5, 0x0b, 0x1a,
        0xa6, 0xe1, 0x39, 0xca, 0xd5, 0x47, 0x5d, 0x3d,
        0xd9, 0x01, 0x5a, 0xd6, 0x51, 0x56, 0x6c, 0x4d,
        0x8b, 0x0d, 0x9a, 0x66, 0xfb, 0xcc, 0xb0, 0x2d,
        0x74, 0x12, 0x2b, 0x20, 0xf0, 0xb1, 0x84, 0x99,
        0xdf, 0x4c, 0xcb, 0xc2, 0x34, 0x7e, 0x76, 0x05,
        0x6d, 0xb7, 0xa9, 0x31, 0xd1, 0x17, 0x04, 0xd7,
        0x14, 0x58, 0x3a, 0x61, 0xde, 0x1b, 0x11, 0x1c,
        0x32, 0x0f, 0x9c, 0x16, 0x53, 0x18, 0xf2, 0x22,
        0xfe, 0x44, 0xcf, 0xb2, 0xc3, 0xb5, 0x7a, 0x91,
        0x24, 0x08, 0xe8, 0xa8, 0x60, 0xfc, 0x69, 0x50,
        0xaa, 0xd0, 0xa0, 0x7d, 0xa1, 0x89, 0x62, 0x97,
        0x54, 0x5b, 0x1e, 0x95, 0xe0, 0xff, 0x64, 0xd2,
        0x10, 0xc4, 0x00, 0x48, 0xa3, 0xf7, 0x75, 0xdb,
        0x8a, 0x03, 0xe6, 0xda, 0x09, 0x3f, 0xdd, 0x94,
        0x87, 0x5c, 0x83, 0x02, 0xcd, 0x4a, 0x90, 0x33,
        0x73, 0x67, 0xf6, 0xf3, 0x9d, 0x7f, 0xbf, 0xe2,
        0x52, 0x9b, 0xd8, 0x26, 0xc8, 0x37, 0xc6, 0x3b,
        0x81, 0x96, 0x6f, 0x4b, 0x13, 0xbe, 0x63, 0x2e,
        0xe9, 0x79, 0xa7, 0x8c, 0x9f, 0x6e, 0xbc, 0x8e,
        0x29, 0xf5, 0xf9, 0xb6, 0x2f, 0xfd, 0xb4, 0x59,
        0x78, 0x98, 0x06, 0x6a, 0xe7, 0x46, 0x71, 0xba,
        0xd4, 0x25, 0xab, 0x42, 0x88, 0xa2, 0x8d, 0xfa,
        0x72, 0x07, 0xb9, 0x55, 0xf8, 0xee, 0xac, 0x0a,
        0x36, 0x49, 0x2a, 0x68, 0x3c, 0x38, 0xf1, 0xa4,
        0x40, 0x28, 0xd3, 0x7b, 0xbb, 0xc9, 0x43, 0xc1,
        0x15, 0xe3, 0xad, 0xf4, 0x77, 0xc7, 0x80, 0x9e
};

// S-box 2
static const uint8_t s2[256] =
{
        0xe0, 0x05, 0x58, 0xd9, 0x67, 0x4e, 0x81, 0xcb,
        0xc9, 0x0b, 0xae, 0x6a, 0xd5, 0x18, 0x5d, 0x82,
        0x46, 0xdf, 0xd6, 0x27, 0x8a, 0x32, 0x4b, 0x42,
        0xdb, 0x1c, 0x9e, 0x9c, 0x3a, 0xca, 0x25, 0x7b,
        0x0d, 0x71, 0x5f, 0x1f, 0xf8, 0xd7, 0x3e, 0x9d,
        0x7c, 0x60, 0xb9, 0xbe, 0xbc, 0x8b, 0x16, 0x34,
        0x4d, 0xc3, 0x72, 0x95, 0xab, 0x8e, 0xba, 0x7a,
        0xb3, 0x02, 0xb4, 0xad, 0xa2, 0xac, 0xd8, 0x9a,
        0x17, 0x1a, 0x35, 0xcc, 0xf7, 0x99, 0x61, 0x5a,
        0xe8, 0x24, 0x56, 0x40, 0xe1, 0x63, 0x09, 0x33,
        0xbf, 0x98, 0x97, 0x85, 0x68, 0xfc, 0xec, 0x0a,
        0xda, 0x6f, 0x53, 0x62, 0xa3, 0x2e, 0x08, 0xaf,
        0x28, 0xb0, 0x74, 0xc2, 0xbd, 0x36, 0x22, 0x38,
        0x64, 0x1e, 0x39, 0x2c, 0xa6, 0x30, 0xe5, 0x44,
        0xfd, 0x88, 0x9f, 0x65, 0x87, 0x6b, 0xf4, 0x23,
        0x48, 0x10, 0xd1, 0x51, 0xc0, 0xf9, 0xd2, 0xa0,
        0x55, 0xa1, 0x41, 0xfa, 0x43, 0x13, 0xc4, 0x2f,
        0xa8, 0xb6, 0x3c, 0x2b, 0xc1, 0xff, 0xc8, 0xa5,
        0x20, 0x89, 0x00, 0x90, 0x47, 0xef, 0xea, 0xb7,
        0x15, 0x06, 0xcd, 0xb5, 0x12, 0x7e, 0xbb, 0x29,
        0x0f, 0xb8, 0x07, 0x04, 0x9b, 0x94, 0x21, 0x66,
        0xe6, 0xce, 0xed, 0xe7, 0x3b, 0xfe, 0x7f, 0xc5,
        0xa4, 0x37, 0xb1, 0x4c, 0x91, 0x6e, 0x8d, 0x76,
        0x03, 0x2d, 0xde, 0x96, 0x26, 0x7d, 0xc6, 0x5c,
        0xd3, 0xf2, 0x4f, 0x19, 0x3f, 0xdc, 0x79, 0x1d,
        0x52, 0xeb, 0xf3, 0x6d, 0x5e, 0xfb, 0x69, 0xb2,
        0xf0, 0x31, 0x0c, 0xd4, 0xcf, 0x8c, 0xe2, 0x75,
        0xa9, 0x4a, 0x57, 0x84, 0x11, 0x45, 0x1b, 0xf5,
        0xe4, 0x0e, 0x73, 0xaa, 0xf1, 0xdd, 0x59, 0x14,
        0x6c, 0x92, 0x54, 0xd0, 0x78, 0x70, 0xe3, 0x49,
        0x80, 0x50, 0xa7, 0xf6, 0x77, 0x93, 0x86, 0x83,
        0x2a, 0xc7, 0x5b, 0xe9, 0xee, 0x8f, 0x01, 0x3d
};

// S-box 3
static const uint8_t s3[256] =
{
        0x38, 0x41, 0x16, 0x76, 0xd9, 0x93, 0x60, 0xf2,
        0x72, 0xc2, 0xab, 0x9a, 0x75, 0x06, 0x57, 0xa0,
        0x91, 0xf7, 0xb5, 0xc9, 0xa2, 0x8c, 0xd2, 0x90,
        0xf6, 0x07, 0xa7, 0x27, 0x8e, 0xb2, 0x49, 0xde,
        0x43, 0x5c, 0xd7, 0xc7, 0x3e, 0xf5, 0x8f, 0x67,
        0x1f, 0x18, 0x6e, 0xaf, 0x2f, 0xe2, 0x85, 0x0d,
        0x53, 0xf0, 0x9c, 0x65, 0xea, 0xa3, 0xae, 0x9e,
        0xec, 0x80, 0x2d, 0x6b, 0xa8, 0x2b, 0x36, 0xa6,
        0xc5, 0x86, 0x4d, 0x33, 0xfd, 0x66, 0x58, 0x96,
        0x3a, 0x09, 0x95, 0x10, 0x78, 0xd8, 0x42, 0xcc,
        0xef, 0x26, 0xe5, 0x61, 0x1a, 0x3f, 0x3b, 0x82,
        0xb6, 0xdb, 0xd4, 0x98, 0xe8, 0x8b, 0x02, 0xeb,
        0x0a, 0x2c, 0x1d, 0xb0, 0x6f, 0x8d, 0x88, 0x0e,
        0x19, 0x87, 0x4e, 0x0b, 0xa9, 0x0c, 0x79, 0x11,
        0x7f, 0x22, 0xe7, 0x59, 0xe1, 0xda, 0x3d, 0xc8,
        0x12, 0x04, 0x74, 0x54, 0x30, 0x7e, 0xb4, 0x28,
        0x55, 0x68, 0x50, 0xbe, 0xd0, 0xc4, 0x31, 0xcb,
        0x2a, 0xad, 0x0f, 0xca, 0x70, 0xff, 0x32, 0x69,
        0x08, 0x62, 0x00, 0x24, 0xd1, 0xfb, 0xba, 0xed,
        0x45, 0x81, 0x73, 0x6d, 0x84, 0x9f, 0xee, 0x4a,
        0xc3, 0x2e, 0xc1, 0x01, 0xe6, 0x25, 0x48, 0x99,
        0xb9, 0xb3, 0x7b, 0xf9, 0xce, 0xbf, 0xdf, 0x71,
        0x29, 0xcd, 0x6c, 0x13, 0x64, 0x9b, 0x63, 0x9d,
        0xc0, 0x4b, 0xb7, 0xa5, 0x89, 0x5f, 0xb1, 0x17,
        0xf4, 0xbc, 0xd3, 0x46, 0xcf, 0x37, 0x5e, 0x47,
        0x94, 0xfa, 0xfc, 0x5b, 0x97, 0xfe, 0x5a, 0xac,
        0x3c, 0x4c, 0x03, 0x35, 0xf3, 0x23, 0xb8, 0x5d,
        0x6a, 0x92, 0xd5, 0x21, 0x44, 0x51, 0xc6, 0x7d,
        0x39, 0x83, 0xdc, 0xaa, 0x7c, 0x77, 0x56, 0x05,
        0x1b, 0xa4, 0x15, 0x34, 0x1e, 0x1c, 0xf8, 0x52,
        0x20, 0x14, 0xe9, 0xbd, 0xdd, 0xe4, 0xa1, 0xe0,
        0x8a, 0xf1, 0xd6, 0x7a, 0xbb, 0xe3, 0x40, 0x4f
};

// S-box 4
static const uint8_t s4[256] =
{
        0x70, 0x2c, 0xb3, 0xc0, 0xe4, 0x57, 0xea, 0xae,
        0x23, 0x6b, 0x45, 0xa5, 0xed, 0x4f, 0x1d, 0x92,
        0x86, 0xaf, 0x7c, 0x1f, 0x3e, 0xdc, 0x5e, 0x0b,
        0xa6, 0x39, 0xd5, 0x5d, 0xd9, 0x5a, 0x51, 0x6c,
        0x8b, 0x9a, 0xfb, 0xb0, 0x74, 0x2b, 0xf0, 0x84,
        0xdf, 0xcb, 0x34, 0x76, 0x6d, 0xa9, 0xd1, 0x04,
        0x14, 0x3a, 0xde, 0x11, 0x32, 0x9c, 0x53, 0xf2,
        0xfe, 0xcf, 0xc3, 0x7a, 0x24, 0xe8, 0x60, 0x69,
        0xaa, 0xa0, 0xa1, 0x62, 0x54, 0x1e, 0xe0, 0x64,
        0x10, 0x00, 0xa3, 0x75, 0x8a, 0xe6, 0x09, 0xdd,
        0x87, 0x83, 0xcd, 0x90, 0x73, 0xf6, 0x9d, 0xbf,
        0x52, 0xd8, 0xc8, 0xc6, 0x81, 0x6f, 0x13, 0x63,
        0xe9, 0xa7, 0x9f, 0xbc, 0x29, 0xf9, 0x2f, 0xb4,
        0x78, 0x06, 0xe7, 0x71, 0xd4, 0xab, 0x88, 0x8d,
        0x72, 0xb9, 0xf8, 0xac, 0x36, 0x2a, 0x3c, 0xf1,
        0x40, 0xd3, 0xbb, 0x43, 0x15, 0xad, 0x77, 0x80,
        0x82, 0xec, 0x27, 0xe5, 0x85, 0x35, 0x0c, 0x41,
        0xef, 0x93, 0x19, 0x21, 0x0e, 0x4e, 0x65, 0xbd,
        0xb8, 0x8f, 0xeb, 0xce, 0x30, 0x5f, 0xc5, 0x1a,
        0xe1, 0xca, 0x47, 0x3d, 0x01, 0xd6, 0x56, 0x4d,
        0x0d, 0x66, 0xcc, 0x2d, 0x12, 0x20, 0xb1, 0x99,
        0x4c, 0xc2, 0x7e, 0x05, 0xb7, 0x31, 0x17, 0xd7,
        0x58, 0x61, 0x1b, 0x1c, 0x0f, 0x16, 0x18, 0x22,
        0x44, 0xb2, 0xb5, 0x91, 0x08, 0xa8, 0xfc, 0x50,
        0xd0, 0x7d, 0x89, 0x97, 0x5b, 0x95, 0xff, 0xd2,
        0xc4, 0x48, 0xf7, 0xdb, 0x03, 0xda, 0x3f, 0x94,
        0x5c, 0x02, 0x4a, 0x33, 0x67, 0xf3, 0x7f, 0xe2,
        0x9b, 0x26, 0x37, 0x3b, 0x96, 0x4b, 0xbe, 0x2e,
        0x79, 0x8c, 0x6e, 0x8e, 0xf5, 0xb6, 0xfd, 0x59,
        0x98, 0x6a, 0x46, 0xba, 0x25, 0x42, 0xa2, 0xfa,
        0x07, 0x55, 0xee, 0x0a, 0x49, 0x68, 0x38, 0xa4,
        0x28, 0x7b, 0xc9, 0xc1, 0xe3, 0xf4, 0xc7, 0x9e
};

// rotates for values larger than 64 are done by swapping array elements
#define rol128(_a, _b, _n){\
        uint64_t _t = _b;\
        (_b) = ((_b) << (_n)) | ((_a) >> (64 - (_n)));\
        (_a) = ((_a) << (_n)) | ((_t) >> (64 - (_n)));\
}

#define expr_rol32_1(_a)\
        (((_a) << 1) | ((_a) >> 31))

uint64_t camellia_naive_S(uint64_t X)
{
        uint64_t result = 0UL;
        result |= (uint64_t)s1[(X >> 56) & 0xff] << 56;
        result |= (uint64_t)s2[(X >> 48) & 0xff] << 48;
        result |= (uint64_t)s3[(X >> 40) & 0xff] << 40;
        result |= (uint64_t)s4[(X >> 32) & 0xff] << 32;
        result |= (uint64_t)s2[(X >> 24) & 0xff] << 24;
        result |= (uint64_t)s3[(X >> 16) & 0xff] << 16;
        result |= (uint64_t)s4[(X >> 8 ) & 0xff] << 8;
        result |= (uint64_t)s1[(X >> 0 ) & 0xff] << 0;

        return result;
}

uint64_t camellia_naive_P(uint64_t X)
{
        uint8_t z1 = X >> 56;
        uint8_t z2 = X >> 48;
        uint8_t z3 = X >> 40;
        uint8_t z4 = X >> 32;
        uint8_t z5 = X >> 24;
        uint8_t z6 = X >> 16;
        uint8_t z7 = X >> 8;
        uint8_t z8 = X >> 0;

        uint64_t result = 0UL;
        result |= (uint64_t)(z1 ^ z3 ^ z4 ^ z6 ^ z7 ^ z8) << 56;
        result |= (uint64_t)(z1 ^ z2 ^ z4 ^ z5 ^ z7 ^ z8) << 48;
        result |= (uint64_t)(z1 ^ z2 ^ z3 ^ z5 ^ z6 ^ z8) << 40;
        result |= (uint64_t)(z2 ^ z3 ^ z4 ^ z5 ^ z6 ^ z7) << 32;
        result |= (uint64_t)(z1 ^ z2 ^ z6 ^ z7 ^ z8) << 24;
        result |= (uint64_t)(z2 ^ z3 ^ z5 ^ z7 ^ z8) << 16;
        result |= (uint64_t)(z3 ^ z4 ^ z5 ^ z6 ^ z8) << 8;
        result |= (uint64_t)(z1 ^ z4 ^ z5 ^ z6 ^ z7) << 0;

        return result;
}

uint64_t camellia_naive_F(uint64_t X, const uint64_t k)
{
        return camellia_naive_P(camellia_naive_S(X ^ k));
}

uint64_t camellia_naive_FL(uint64_t X, const uint64_t kl)
{
        const uint32_t XL = (X >> 32);
        const uint32_t XR = (X >> 0);

        const uint32_t klL = (kl >> 32);
        const uint32_t klR = (kl >> 0);

        const uint32_t YR = expr_rol32_1(XL & klL) ^ XR;
        const uint32_t YL = (YR | klR) ^ XL;

        return ((uint64_t)YL << 32) | (uint64_t)YR;
}

uint64_t camellia_naive_FL_inv(uint64_t Y, const uint64_t kl)
{
        const uint32_t YL = (Y >> 32);
        const uint32_t YR = (Y >> 0);

        const uint32_t klL = (kl >> 32);
        const uint32_t klR = (kl >> 0);

        const uint32_t XL = (YR | klR) ^ YL;
        const uint32_t XR = expr_rol32_1(XL & klL) ^ YR;

        return ((uint64_t)XL << 32) | (uint64_t)XR;
}

void camellia_naive_feistel_round(uint64_t state[2], const uint64_t kr)
{
        const uint64_t Lr = state[1] ^ camellia_naive_F(state[0], kr);
        state[1] = state[0];
        state[0] = Lr;
}

void camellia_naive_feistel_round_inv(uint64_t state[2], const uint64_t kr)
{
        const uint64_t Rr1 = state[0] ^ camellia_naive_F(state[1], kr);
        state[0] = state[1];
        state[1] = Rr1;
}

void camellia_naive_generate_round_keys_128(const uint64_t key[restrict 2],
                                        struct camellia_rks_128 *restrict rks)
{
        uint64_t KL[2], KA[2];
        memcpy(KL, key, sizeof(KL));
        memcpy(KA, key, sizeof(KA));

        // compute KA
        camellia_naive_feistel_round(KA, keysched_const[0]);
        camellia_naive_feistel_round(KA, keysched_const[1]);
        KA[0] ^= KL[0], KA[1] ^= KL[1];
        camellia_naive_feistel_round(KA, keysched_const[2]);
        camellia_naive_feistel_round(KA, keysched_const[3]);

        // KB not needed for 128 bit

        // pre-whitening
        rks->kw[0] = KL[0]; rks->kw[1] = KL[1];

        // feistel rounds 0-5
        rks->ku[0] = KA[0]; rks->ku[1] = KA[1];
        rol128(KL[0], KL[1], 15); // KL << 15
        rks->ku[2] = KL[0]; rks->ku[3] = KL[1];
        rol128(KA[0], KA[1], 15); // KA << 15
        rks->ku[4] = KA[0]; rks->ku[5] = KA[1];

        // FL/FL-1
        rol128(KA[0], KA[1], 15); // KA << 30
        rks->kl[0] = KA[0]; rks->kl[1] = KA[1];

        // feistel rounds 6-11
        rol128(KL[0], KL[1], 30) // KL << 45
                rks->ku[6] = KL[0]; rks->ku[7] = KL[1];
        rol128(KA[0], KA[1], 15) // KA << 45
                rks->ku[8] = KA[0];
        rol128(KL[0], KL[1], 15) // KL << 60
                rks->ku[9] = KL[1];
        rol128(KA[0], KA[1], 15) // KA << 60
                rks->ku[10] = KA[0]; rks->ku[11] = KA[1];

        // FL/FL-1
        rol128(KL[0], KL[1], 17) // KL << 77
                rks->kl[2] = KL[0]; rks->kl[3] = KL[1];

        // feistel rounds 12-17
        rol128(KL[0], KL[1], 17) // KL << 94
                rks->ku[12] = KL[0]; rks->ku[13] = KL[1];
        rol128(KA[0], KA[1], 34) // KA << 94
                rks->ku[14] = KA[0]; rks->ku[15] = KA[1];
        rol128(KL[0], KL[1], 17) // KL << 111
                rks->ku[16] = KL[0]; rks->ku[17] = KL[1];

        // post-whitening
        rol128(KA[0], KA[1], 17) // KA << 111
                rks->kw[2] = KA[0]; rks->kw[3] = KA[1];
}

void camellia_naive_generate_round_keys_256(const uint64_t key[restrict 4],
                                            struct camellia_rks_256 *restrict rks)
{
        uint64_t KL[2], KR[2], KA[2], KB[2];
        memcpy(KL, &key[0], sizeof(KL));
        memcpy(KR, &key[2], sizeof(KR));
        memcpy(KA, KL, sizeof(KA));

        // compute KA
        KA[0] ^= KR[0]; KA[1] ^= KR[1];
        camellia_naive_feistel_round(KA, keysched_const[0]);
        camellia_naive_feistel_round(KA, keysched_const[1]);
        KA[0] ^= KL[0], KA[1] ^= KL[1];
        camellia_naive_feistel_round(KA, keysched_const[2]);
        camellia_naive_feistel_round(KA, keysched_const[3]);

        // compute KB
        memcpy(KB, KA, sizeof(KB));
        KB[0] ^= KR[0]; KB[1] ^= KR[1];
        camellia_naive_feistel_round(KB, keysched_const[4]);
        camellia_naive_feistel_round(KB, keysched_const[5]);

        // pre-whitening
        rks->kw[0] = KL[0]; rks->kw[1] = KL[1];

        // feistel rounds 0-5
        rks->ku[0] = KB[0]; rks->ku[1] = KB[1];
        rol128(KR[0], KR[1], 15); // KR << 15
        rks->ku[2] = KR[0]; rks->ku[3] = KR[1];
        rol128(KA[0], KA[1], 15); // KA << 15
        rks->ku[4] = KA[0]; rks->ku[5] = KA[1];

        // FL/FL-1
        rol128(KR[0], KR[1], 15); // KR << 30
        rks->kl[0] = KR[0]; rks->kl[1] = KR[1];

        // feistel rounds 6-11
        rol128(KB[0], KB[1], 30) // KB << 30
                rks->ku[6] = KB[0]; rks->ku[7] = KB[1];
        rol128(KL[0], KL[1], 45) // KL << 45
                rks->ku[8] = KL[0]; rks->ku[9] = KL[1];
        rol128(KA[0], KA[1], 30) // KA << 45
                rks->ku[10] = KA[0]; rks->ku[11] = KA[1];

        // FL/FL-1
        rol128(KL[0], KL[1], 15) // KL << 60
                rks->kl[2] = KL[0]; rks->kl[3] = KL[1];

        // feistel rounds 12-17
        rol128(KR[0], KR[1], 30) // KR << 60
                rks->ku[12] = KR[0]; rks->ku[13] = KR[1];
        rol128(KB[0], KB[1], 30) // KB << 60
                rks->ku[14] = KB[0]; rks->ku[15] = KB[1];
        rol128(KL[0], KL[1], 17) // KL << 77
                rks->ku[16] = KL[0]; rks->ku[17] = KL[1];

        // FL/FL-1
        rol128(KA[0], KA[1], 32) // KA << 77
                rks->kl[4] = KA[0]; rks->kl[5] = KA[1];

        // feistel rounds 18-25
        rol128(KR[0], KR[1], 34) // KR << 94
                rks->ku[18] = KR[0]; rks->ku[19] = KR[1];
        rol128(KA[0], KA[1], 17) // KA << 94
                rks->ku[20] = KA[0]; rks->ku[21] = KA[1];
        rol128(KL[0], KL[1], 34) // KL << 111
                rks->ku[22] = KL[0]; rks->ku[23] = KL[1];

        // post-whitening
        rol128(KB[0], KB[1], 51) // KB << 111
                rks->kw[2] = KB[0]; rks->kw[3] = KB[1];
}

void camellia_naive_encrypt_128(uint64_t c[restrict 2],
                            const uint64_t m[restrict 2],
                            struct camellia_rks_128 *restrict rks)
{
        memcpy(c, m, sizeof(c[0]) * 2);

        c[0] ^= rks->kw[0]; c[1] ^= rks->kw[1];

        for (int i = 0; i < 6; i++) {
                camellia_naive_feistel_round(c, rks->ku[i + 0]);
        }

        c[0] = camellia_naive_FL(c[0], rks->kl[0]);
        c[1] = camellia_naive_FL_inv(c[1], rks->kl[1]);

        for (int i = 0; i < 6; i++) {
                camellia_naive_feistel_round(c, rks->ku[i + 6]);
        }

        c[0] = camellia_naive_FL(c[0], rks->kl[2]);
        c[1] = camellia_naive_FL_inv(c[1], rks->kl[3]);

        for (int i = 0; i < 6; i++) {
                camellia_naive_feistel_round(c, rks->ku[i + 12]);
        }

        // swap c[0] and c[1] (concatenation of R||L)
        uint64_t t = c[0];
        c[0] = c[1]; c[1] = t;

        c[0] ^= rks->kw[2]; c[1] ^= rks->kw[3];
}

void camellia_naive_decrypt_128(uint64_t m[restrict 2],
                            const uint64_t c[restrict 2],
                            struct camellia_rks_128 *restrict rks)
{
        memcpy(m, c, sizeof(m[0]) * 2);

        m[0] ^= rks->kw[2]; m[1] ^= rks->kw[3];

        // swap c[0] and c[1] (concatenation of R||L)
        uint64_t t = m[0];
        m[0] = m[1]; m[1] = t;

        for (int i = 5; i >= 0; i--) {
                camellia_naive_feistel_round_inv(m, rks->ku[i + 12]);
        }

        m[1] = camellia_naive_FL(m[1], rks->kl[3]);
        m[0] = camellia_naive_FL_inv(m[0], rks->kl[2]);

        for (int i = 5; i >= 0; i--) {
                camellia_naive_feistel_round_inv(m, rks->ku[i + 6]);
        }

        m[1] = camellia_naive_FL(m[1], rks->kl[1]);
        m[0] = camellia_naive_FL_inv(m[0], rks->kl[0]);

        for (int i = 5; i >= 0; i--) {
                camellia_naive_feistel_round_inv(m, rks->ku[i + 0]);
        }

        m[0] ^= rks->kw[0]; m[1] ^= rks->kw[1];

}

void camellia_naive_encrypt_256(uint64_t c[restrict 2],
                            const uint64_t m[restrict 2],
                            struct camellia_rks_256 *restrict rks)
{
        memcpy(c, m, sizeof(c[0]) * 2);

        c[0] ^= rks->kw[0]; c[1] ^= rks->kw[1];

        for (int i = 0; i < 6; i++) {
                camellia_naive_feistel_round(c, rks->ku[i + 0]);
        }

        c[0] = camellia_naive_FL(c[0], rks->kl[0]);
        c[1] = camellia_naive_FL_inv(c[1], rks->kl[1]);

        for (int i = 0; i < 6; i++) {
                camellia_naive_feistel_round(c, rks->ku[i + 6]);
        }

        c[0] = camellia_naive_FL(c[0], rks->kl[2]);
        c[1] = camellia_naive_FL_inv(c[1], rks->kl[3]);

        for (int i = 0; i < 6; i++) {
                camellia_naive_feistel_round(c, rks->ku[i + 12]);
        }

        c[0] = camellia_naive_FL(c[0], rks->kl[4]);
        c[1] = camellia_naive_FL_inv(c[1], rks->kl[5]);

        for (int i = 0; i < 6; i++) {
                camellia_naive_feistel_round(c, rks->ku[i + 18]);
        }

        // swap c[0] and c[1] (concatenation of R||L)
        uint64_t t = c[0];
        c[0] = c[1]; c[1] = t;

        c[0] ^= rks->kw[2]; c[1] ^= rks->kw[3];
}

void camellia_naive_decrypt_256(uint64_t m[restrict 2],
                            const uint64_t c[restrict 2],
                            struct camellia_rks_256 *restrict rks)
{
        memcpy(m, c, sizeof(m[0]) * 2);

        m[0] ^= rks->kw[2]; m[1] ^= rks->kw[3];

        // swap c[0] and c[1] (concatenation of R||L)
        uint64_t t = m[0];
        m[0] = m[1]; m[1] = t;

        for (int i = 5; i >= 0; i--) {
                camellia_naive_feistel_round_inv(m, rks->ku[i + 18]);
        }

        m[1] = camellia_naive_FL(m[1], rks->kl[5]);
        m[0] = camellia_naive_FL_inv(m[0], rks->kl[4]);

        for (int i = 5; i >= 0; i--) {
                camellia_naive_feistel_round_inv(m, rks->ku[i + 12]);
        }

        m[1] = camellia_naive_FL(m[1], rks->kl[3]);
        m[0] = camellia_naive_FL_inv(m[0], rks->kl[2]);

        for (int i = 5; i >= 0; i--) {
                camellia_naive_feistel_round_inv(m, rks->ku[i + 6]);
        }

        m[1] = camellia_naive_FL(m[1], rks->kl[1]);
        m[0] = camellia_naive_FL_inv(m[0], rks->kl[0]);

        for (int i = 5; i >= 0; i--) {
                camellia_naive_feistel_round_inv(m, rks->ku[i + 0]);
        }

        m[0] ^= rks->kw[0]; m[1] ^= rks->kw[1];
}
