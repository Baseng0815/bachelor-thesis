#include "gift.h"
#include "gift_sliced.h"
#include "gift_neon.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// no testing framework necessary for this small project
#define ASSERT_EQUALS(x,y)\
        if (x != y) {\
                fprintf(stderr, "ASSERT_EQUALS failed on line %d: %lx != %lx\n", __LINE__, x, y);\
                exit(-1);\
        }

void key_rand(uint64_t k[])
{
        k[0] = rand();
        k[1] = rand();
}

void m_rand(uint64_t m[], size_t n)
{
        for (size_t i = 0; i < n; i++) {
                m[i] = rand() | ((uint64_t)rand() << 32);
        }
}

void test_gift_64(void)
{
        printf("testing GIFT_64...\n");

        // test encrypt to known value
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m = 0x4dcfd3bdd61810f0UL;
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c = gift_64_encrypt(m, key);
        ASSERT_EQUALS(c, c_expected);

        // test encrypt-decrypt
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand(&m, 1);

                c = gift_64_encrypt(m, key);
                uint64_t m_actual = gift_64_decrypt(c, key);
                printf("m=%016lx, c=%016lx, m1=%016lx\n", m, c, m_actual);
                ASSERT_EQUALS(m, m_actual);
        }
}

void test_gift_128(void)
{
        printf("testing GIFT_128...\n");

        // test encrypt to known value
        uint64_t key[2] = { 0xe4cb97525e4e9ec7UL, 0xac22df007be1e75fUL };
        uint64_t m[2] = { 0xb56727d75611fa52UL, 0xf22d8525d7ce8033UL };
        uint64_t c_expected[2] = {0x781e0899b0630487UL, 0xe81f38a697c07b06UL };
        uint64_t c[2];
        gift_128_encrypt(c, m, key);
        ASSERT_EQUALS(c[0], c_expected[0]);
        ASSERT_EQUALS(c[1], c_expected[1]);

        // test encrypt-decrypt
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand(m, 2);

                gift_128_encrypt(c, m, key);
                uint64_t m_actual[2];
                gift_128_decrypt(m_actual, c, key);
                printf("m=[%016lx, %016lx], c=[%016lx, %016lx], m1=[%016lx, %016lx]\n",
                       m[0], m[1], c[0], c[1], m_actual[0], m_actual[1]);
                ASSERT_EQUALS(m[0], m_actual[0]);
                ASSERT_EQUALS(m[1], m_actual[1]);
        }
}

void test_gift_64_sliced(void)
{
        printf("testing GIFT_64_SLICED...\n");

        // test encrypt to known value (8 times the same)
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
                ASSERT_EQUALS(c[i], c_expected);
        }

        // test encrypt-decrypt
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand(m, 8);

                gift_64_sliced_encrypt(c, m, key);
                uint64_t m_actual[8];
                gift_64_sliced_decrypt(m_actual, c, key);
                printf("m=[%016lx, %016lx, ...], c=[%016lx, %016lx, ...], m1=[%016lx, %016lx, ...]\n",
                       m[0], m[1], c[0], c[1], m_actual[0], m_actual[1]);
                for (size_t j = 0; j < 8; j++) {
                        ASSERT_EQUALS(m[j], m_actual[j]);
                }
        }
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        test_gift_64();
        test_gift_128();
        test_gift_64_sliced();
}
