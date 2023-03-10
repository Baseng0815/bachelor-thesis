#pragma clang optimize off

#include "naive/gift.h"
#include "naive/gift_sliced.h"
#include "table/gift_table.h"
#include "vector/gift_vec_sbox.h"
#include "vector/gift_vec_sliced.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <stdint.h>
#include <arm_neon.h>

#define MEASURE(code, t0, t1)\
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t0));\
        code;\
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t1));\
        printf(#code ": took %ld cycles\n", t1 - t0);

void key_rand(uint64_t k[])
{
        k[0] = rand();
        k[1] = rand();
}

void m_rand(uint8_t m[], size_t n)
{
        for (size_t i = 0; i < n; i++) {
                m[i] = rand();
        }
}

void benchmark_gift_64(void)
{
        uint64_t key[2];
        key_rand(key);

        uint64_t m;
        m_rand((uint8_t*)&m, sizeof(m));

        uint64_t round_keys[ROUNDS_GIFT_64];

        uint64_t t0, t1;
        // TODO take into account repeated execution due to cache effects
        MEASURE(gift_64_generate_round_keys(round_keys, key), t0, t1);
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_64_subcells(m), t0, t1);
        }
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_64_permute(m), t0, t1);
        }
        MEASURE(gift_64_encrypt(m, key), t0, t1);
}

void benchmark_gift_128(void)
{
        uint64_t key[2];
        key_rand(key);

        uint8_t m[16];
        m_rand(m, sizeof(m));

        uint8_t c[16];
        uint8_t round_keys[ROUNDS_GIFT_128][32];

        uint64_t t0, t1;
        // TODO take into account repeated execution due to cache effects
        MEASURE(gift_128_generate_round_keys(round_keys, key), t0, t1);
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_128_subcells(m), t0, t1);
        }
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_128_permute(m), t0, t1);
        }
        MEASURE(gift_128_encrypt(c, m, key), t0, t1);
}

void benchmark_gift_64_sliced(void)
{
        uint64_t key[2];
        key_rand(key);

        uint8_t m[16];
        m_rand(m, sizeof(m));

        uint8_t c[16];
        uint8_t round_keys[ROUNDS_GIFT_128][32];

        uint64_t t0, t1;
        // TODO take into account repeated execution due to cache effects
        MEASURE(gift_64_sliced_generate_round_keys(round_keys, key), t0, t1);
        MEASURE(gift_64_sliced_subcells(m), t0, t1);
        MEASURE(gift_64_sliced_permute(m), t0, t1);
        MEASURE(gift_64_sliced_encrypt(c, m, key), t0, t1);
}

void benchmark_gift_64_vec_sbox(void)
{
        uint64_t key[2];
        key_rand(key);
        gift_64_vec_sbox_init();

        uint8x16_t m;
        m = vsetq_lane_u32(rand(), m, 0);
        m = vsetq_lane_u32(rand(), m, 1);
        m = vsetq_lane_u32(rand(), m, 2);
        m = vsetq_lane_u32(rand(), m, 3);

        uint8x16_t round_keys[ROUNDS_GIFT_64];

        uint64_t t0, t1;
        // TODO take into account repeated execution due to cache effects
        MEASURE(gift_64_vec_sbox_generate_round_keys(round_keys, key), t0, t1);
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_64_vec_sbox_subcells(m), t0, t1);
        }
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_64_vec_sbox_permute(m), t0, t1);
        }

        uint64_t m_;
        m_rand((uint8_t*)&m_, sizeof(m_));
        MEASURE(gift_64_vec_sbox_encrypt(m_, key), t0, t1);
}

void benchmark_gift_64_vec_sliced(void)
{
        uint64_t key[2];
        key_rand(key);

        uint64_t m[16];
        m_rand((uint8_t*)&m, sizeof(m));

        uint64_t c[16];
        uint8x16x4_t round_keys[ROUNDS_GIFT_128][2];
        uint8x16x4_t s[2];
        s[0].val[0] = vdupq_n_u8(0x37);
        s[0].val[1] = vdupq_n_u8(0x29);
        s[0].val[2] = vdupq_n_u8(0x6e);
        s[0].val[3] = vdupq_n_u8(0xcd);
        s[1].val[0] = vdupq_n_u8(0xbe);
        s[1].val[1] = vdupq_n_u8(0x81);
        s[1].val[2] = vdupq_n_u8(0x93);
        s[1].val[3] = vdupq_n_u8(0x01);

        uint64_t t0, t1;
        // TODO take into account repeated execution due to cache effects
        MEASURE(gift_64_vec_sliced_generate_round_keys(round_keys, key), t0, t1);
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_64_vec_sliced_subcells(s), t0, t1);
        }
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_64_vec_sliced_permute(s), t0, t1);
        }

        MEASURE(gift_64_vec_sliced_encrypt(c, m, key), t0, t1);
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        benchmark_gift_64();
        benchmark_gift_128();
        benchmark_gift_64_vec_sbox();
        benchmark_gift_64_vec_sliced();
}

#pragma clang optimize on
