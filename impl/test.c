#pragma clang optimize off

#include "gift/naive.h"
#include "gift/sliced.h"
#include "gift/table.h"
#include "gift/vec_sbox.h"
#include "gift/vec_sliced.h"

#include "camellia/naive.h"
#include "camellia/spec_opt.h"
#include "camellia/bytesliced.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

// no testing framework necessary for this small project
#define ASSERT_EQUALS(x, y)\
        if (x != y) {\
                fprintf(stderr, "ASSERT_EQUALS failed on line %d: %lx != %lx\n", __LINE__, x, y);\
                exit(-1);\
        }

#define ASSERT_TRUE(x)\
        if (!(x)) {\
                fprintf(stderr, "ASSERT_TRUE failed on line %d", __LINE__);\
                exit(-1);\
        }

void m_rand(uint8_t m[], size_t n)
{
        for (size_t i = 0; i < n; i++) {
                m[i] = rand();
        }
}

void test_gift_64(void)
{
        // test encrypt to known value
        printf("testing GIFT_64 encrytion to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m = 0x4dcfd3bdd61810f0UL;
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c;

        uint64_t rks[ROUNDS_GIFT_64];
        gift_64_generate_round_keys(rks, key);
        c = gift_64_encrypt(m, rks);
        ASSERT_EQUALS(c, c_expected);
        return;

        // test encrypt-decrypt
        printf("testing GIFT_64 encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)key, sizeof(key));
                m_rand((uint8_t*)&m, 8);
                memset(&m, 0, sizeof(m));

                gift_64_generate_round_keys(rks, key);
                c = gift_64_encrypt(m, rks);
                uint64_t m_actual = gift_64_decrypt(c, rks);
                ASSERT_EQUALS(m, m_actual);
        }
}

void test_gift_128(void)
{
        // test encrypt to known value
        printf("testing GIFT_128 encryption to known value...\n");
        uint64_t key[2] = { 0xe4cb97525e4e9ec7UL, 0xac22df007be1e75fUL };
        uint8_t m[16] = {
                0x52, 0xfa, 0x11, 0x56, 0xd7, 0x27, 0x67, 0xb5,
                0x33, 0x80, 0xce, 0xd7, 0x25, 0x85, 0x2d, 0xf2
        };
        uint8_t c_expected[16] = {
                0x87, 0x04, 0x63, 0xb0, 0x99, 0x08, 0x1e, 0x78,
                0x06, 0x7b, 0xc0, 0x97, 0xa6, 0x38, 0x1f, 0xe8
        };
        uint8_t c[16];

        uint8_t rks[ROUNDS_GIFT_128][32];
        gift_128_generate_round_keys(rks, key);
        gift_128_encrypt(c, m, rks);
        ASSERT_TRUE(memcmp(c, c_expected, 16) == 0);

        // test encrypt-decrypt
        printf("testing GIFT_128 encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)key, sizeof(key));
                m_rand(m, 16);

                gift_128_generate_round_keys(rks, key);
                gift_128_encrypt(c, m, rks);
                uint8_t m_actual[16];
                gift_128_decrypt(m_actual, c, rks);
                ASSERT_TRUE(memcmp(m, m_actual, 8) == 0);
        }
}

void test_gift_64_sliced(void)
{
        // test encrypt to known value (8 times the same)
        printf("testing GIFT_64_SLICED encryption to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m[8] = {
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL
        };
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c[8];
        gift_64_sliced_encrypt(c, m, key);
        for (size_t i = 0; i < 8; i++) {
                ASSERT_TRUE(memcmp(&c[i], &c_expected, 8) == 0);
        }

        // test encrypt-decrypt
        printf("testing GIFT_64_SLICED encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)key, sizeof(key));
                for (size_t j = 0; j < 8; j++) {
                        m_rand((uint8_t*)&m, sizeof(m));
                }

                gift_64_sliced_encrypt(c, m, key);
                uint64_t m_actual[8];
                gift_64_sliced_decrypt(m_actual, c, key);
                ASSERT_TRUE(memcmp(m_actual, m, 8 * 8) == 0);
        }
}

void test_gift_64_table(void)
{
        return;
        // test encrypt to known value
        printf("testing GIFT_64_TABLE encrytion to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m = 0x4dcfd3bdd61810f0UL;
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c;
        c = gift_64_table_encrypt(m, key);
        ASSERT_EQUALS(c, c_expected);

        // test encrypt-decrypt
        printf("testing GIFT_64_TABLE encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)key, sizeof(key));
                m_rand((uint8_t*)&m, sizeof(m));

                c = gift_64_table_encrypt(m, key);
                // only encryption for table approach
                uint64_t m_actual = gift_64_decrypt(c, key);
                ASSERT_EQUALS(m, m_actual);
        }
}

void test_gift_64_vec_sbox(void)
{
        gift_64_vec_sbox_init();

        // test encrypt to known value
        printf("testing GIFT_64_VEC_SBOX encrytion to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m = 0x4dcfd3bdd61810f0UL;
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c;

        uint8x16_t rks[ROUNDS_GIFT_64];
        gift_64_vec_sbox_generate_round_keys(rks, key);
        c = gift_64_vec_sbox_encrypt(m, rks);
        ASSERT_EQUALS(c, c_expected);

        // test encrypt-decrypt
        printf("testing GIFT_64_VEC_SBOX encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)key, sizeof(key));
                m_rand((uint8_t*)&m, sizeof(m));

                gift_64_vec_sbox_generate_round_keys(rks, key);
                c = gift_64_vec_sbox_encrypt(m, rks);
                // only encryption for table approach
                uint64_t m_actual = gift_64_vec_sbox_decrypt(c, rks);
                ASSERT_EQUALS(m, m_actual);
        }
}

void test_gift_64_vec_sliced(void)
{
        gift_64_vec_sliced_init();

        printf("testing GIFT_64_VEC_SLICED packing/unpacking...\n");
        uint64_t a[8] = { 0UL };
        uint8x16x4_t s[2] = {
                vld1q_u8_x4((uint8_t*)a), vld1q_u8_x4((uint8_t*)a),
        };
        s[0].val[0] = vdupq_n_u64(0x0123456789abcdefUL);
        s[0].val[1] = vdupq_n_u64(0x0123456789abcdefUL);
        s[0].val[2] = vdupq_n_u64(0x0123456789abcdefUL);
        s[0].val[3] = vdupq_n_u64(0x0123456789abcdefUL);
        s[1].val[0] = vdupq_n_u64(0x0123456789abcdefUL);
        s[1].val[1] = vdupq_n_u64(0x0123456789abcdefUL);
        s[1].val[2] = vdupq_n_u64(0x0123456789abcdefUL);
        s[1].val[3] = vdupq_n_u64(0x0123456789abcdefUL);
        gift_64_vec_sliced_bits_pack(s);
        gift_64_vec_sliced_bits_unpack(s);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[0], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[0], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[1], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[1], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[2], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[2], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[3], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[3], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[0], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[0], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[1], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[1], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[2], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[2], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[3], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[3], 1), 0x0123456789abcdefUL);

        printf("testing GIFT_64_VEC_SLICED substitution...\n");
        gift_64_vec_sliced_bits_pack(s);
        gift_64_vec_sliced_subcells(s);
        gift_64_vec_sliced_bits_unpack(s);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[0], 0), 0x1a4c6f392db7508eUL);

        // test encrypt to known value
        printf("testing GIFT_64_VEC_SLICED encrytion to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m[16] = {
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
        };
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c[16];
        uint8x16x4_t rks[ROUNDS_GIFT_64][2];
        gift_64_vec_sliced_generate_round_keys(rks, key);
        gift_64_vec_sliced_encrypt(c, m, rks);
        ASSERT_EQUALS(c[0], c_expected);
        ASSERT_EQUALS(c[1], c_expected);

        // test encrypt-decrypt
        printf("testing GIFT_64_VEC_SLICED encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)key, sizeof(key));
                m_rand((uint8_t*)m, sizeof(m));
                memset(m, 0, sizeof(m));

                gift_64_vec_sliced_generate_round_keys(rks, key);
                gift_64_vec_sliced_encrypt(c, m, rks);
                // only encryption for table approach
                uint64_t m_actual[16];
                gift_64_vec_sliced_decrypt(m_actual, c, rks);
                ASSERT_TRUE(memcmp(m, m_actual, sizeof(m_actual)) == 0);
        }
}

void test_camellia_naive(void)
{
        uint64_t m[2], c[2];
        uint64_t key[2];
        struct camellia_rks_128 rks_128;
        struct camellia_rks_256 rks_256;

        printf("testing CAMELLIA_NAIVE FL-FL_inv and feistel-feistel_inv...\n");
        for (int i = 0; i < 256; i++) {
                m_rand((uint8_t*)m, sizeof(m));
                m_rand((uint8_t*)key, sizeof(key));
                ASSERT_EQUALS(camellia_naive_FL(camellia_spec_opt_FL_inv(m[0], key[0]), key[0]), m[0]);
                ASSERT_EQUALS(camellia_naive_FL_inv(camellia_spec_opt_FL(m[1], key[1]), key[1]), m[1]);

                uint64_t mc[2];
                memcpy(mc, m, sizeof(mc));
                camellia_naive_feistel_round(mc, key[0]);
                camellia_naive_feistel_round_inv(mc, key[0]);
                ASSERT_TRUE(memcmp(mc, m, sizeof(mc)) == 0);
                camellia_naive_feistel_round_inv(mc, key[1]);
                camellia_naive_feistel_round(mc, key[1]);
                ASSERT_TRUE(memcmp(mc, m, sizeof(mc)) == 0);
        }

        printf("testing CAMELLIA_NAIVE 128-bit encrypt to known value...\n");
        key[0] = m[0] = 0x0123456789abcdefUL;
        key[1] = m[1] = 0xfedcba9876543210UL;
        uint64_t c_expected[2] = {
                0x6767313854966973, 0x0857065648eabe43
        };

        camellia_naive_generate_round_keys_128(&rks_128, key);
        camellia_naive_encrypt_128(c, m, &rks_128);
        ASSERT_EQUALS(c[0], c_expected[0]);
        ASSERT_EQUALS(c[1], c_expected[1]);

        printf("testing CAMELLIA_NAIVE 128-bit encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)m, sizeof(m));
                m_rand((uint8_t*)key, sizeof(key));
                camellia_naive_generate_round_keys_128(&rks_128, key);

                uint64_t m_decr[2];
                camellia_naive_encrypt_128(c, m, &rks_128);
                camellia_naive_decrypt_128(m_decr, c, &rks_128);
                ASSERT_EQUALS(m[0], m_decr[0]);
                ASSERT_EQUALS(m[1], m_decr[1]);
        }

        printf("testing CAMELLIA_NAIVE 256-bit encrypt to known value...\n");
        uint64_t key_256[4] = {
                0x0123456789abcdefUL, 0xfedcba9876543210UL,
                0x0011223344556677UL, 0x8899aabbccddeeffUL
        };

        m[0] = 0x0123456789abcdefUL;
        m[1] = 0xfedcba9876543210UL;
        c_expected[0] = 0x9acc237dff16d76cUL;
        c_expected[1] = 0x20ef7c919e3a7509UL;

        camellia_naive_generate_round_keys_256(&rks_256, key_256);
        camellia_naive_encrypt_256(c, m, &rks_256);
        ASSERT_EQUALS(c[0], c_expected[0]);
        ASSERT_EQUALS(c[1], c_expected[1]);

        printf("testing CAMELLIA_NAIVE 256-bit encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)m, sizeof(m));
                m_rand((uint8_t*)key_256, sizeof(key_256));
                camellia_naive_generate_round_keys_256(&rks_256, key);

                uint64_t m_decr[2];
                camellia_naive_encrypt_256(c, m, &rks_256);
                camellia_naive_decrypt_256(m_decr, c, &rks_256);
                ASSERT_EQUALS(m[0], m_decr[0]);
                ASSERT_EQUALS(m[1], m_decr[1]);
        }
}

void test_camellia_spec_opt(void)
{
        uint64_t m[2], c[2];
        uint64_t key[2];
        struct camellia_rks_128 rks_128;

        printf("testing CAMELLIA_SPEC_OPT FL-FL_inv and feistel-feistel_inv...\n");
        for (int i = 0; i < 256; i++) {
                m_rand((uint8_t*)m, sizeof(m));
                m_rand((uint8_t*)key, sizeof(key));
                ASSERT_EQUALS(camellia_spec_opt_FL(camellia_spec_opt_FL_inv(m[0], key[0]), key[0]), m[0]);
                ASSERT_EQUALS(camellia_spec_opt_FL_inv(camellia_spec_opt_FL(m[1], key[1]), key[1]), m[1]);

                uint64_t mc[2];
                memcpy(mc, m, sizeof(mc));
                camellia_spec_opt_feistel_round(mc, key[0]);
                camellia_spec_opt_feistel_round_inv(mc, key[0]);
                ASSERT_TRUE(memcmp(mc, m, sizeof(mc)) == 0);
                camellia_spec_opt_feistel_round_inv(mc, key[1]);
                camellia_spec_opt_feistel_round(mc, key[1]);
                ASSERT_TRUE(memcmp(mc, m, sizeof(mc)) == 0);
        }

        printf("testing CAMELLIA_SPEC_OPT 128-bit encrypt to known value...\n");
        key[0] = m[0] = 0x0123456789abcdefUL;
        key[1] = m[1] = 0xfedcba9876543210UL;
        uint64_t c_expected[2] = {
                0x6767313854966973, 0x0857065648eabe43
        };

        camellia_spec_opt_generate_round_keys_128(&rks_128, key);
        camellia_spec_opt_encrypt_128(c, m, &rks_128);
        ASSERT_EQUALS(c[0], c_expected[0]);
        ASSERT_EQUALS(c[1], c_expected[1]);

        printf("testing CAMELLIA_SPEC_OPT 128-bit encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)m, sizeof(m));
                m_rand((uint8_t*)key, sizeof(key));
                camellia_spec_opt_generate_round_keys_128(&rks_128, key);

                uint64_t m_decr[2];
                camellia_spec_opt_encrypt_128(c, m, &rks_128);
                camellia_spec_opt_decrypt_128(m_decr, c, &rks_128);
                ASSERT_EQUALS(m[0], m_decr[0]);
                ASSERT_EQUALS(m[1], m_decr[1]);
        }
}

void test_camellia_sliced(void)
{
        printf("testing CAMELLIA_SLICED 128-bit encrypt to known value...\n");

        camellia_sliced_init();

        uint64_t key[2] = {
                0x0123456789abcdefUL, 0xfedcba9876543210UL
        };

        uint64_t m[16][2];
        uint64_t c[16][2];
        for (size_t i = 0; i < 16; i++) {
                m[i][0] = 0x0123456789abcdefUL;
                m[i][1] = 0xfedcba9876543210UL;
        }

        uint64_t c_expected[2] = {
                0x6767313854966973, 0x0857065648eabe43
        };

        struct camellia_rks_sliced_128 rks;
        camellia_sliced_generate_round_keys_128(&rks, key);
        camellia_sliced_encrypt_128(c, m, &rks);

        for (size_t i = 0; i < 16; i++) {
                ASSERT_TRUE(memcmp(c_expected, &c[i], sizeof(c_expected)) == 0);
        }

        printf("testing CAMELLIA_SLICED 128-bit encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)m, sizeof(m));
                m_rand((uint8_t*)key, sizeof(key));
                camellia_sliced_generate_round_keys_128(&rks, key);

                uint64_t m_decr[16][2];
                camellia_sliced_encrypt_128(c, m, &rks);
                camellia_sliced_decrypt_128(m_decr, c, &rks);
                for (size_t i = 0; i < 16; i++) {
                        ASSERT_EQUALS(m_decr[i][0], m[i][0]);
                        ASSERT_EQUALS(m_decr[i][1], m[i][1]);
                }
        }
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        test_gift_64();
        test_gift_128();
        test_gift_64_sliced();
        test_gift_64_table();
        test_gift_64_vec_sbox();
        test_gift_64_vec_sliced();
        test_camellia_naive();
        test_camellia_spec_opt();
        test_camellia_sliced();
}

#pragma clang optimize on
