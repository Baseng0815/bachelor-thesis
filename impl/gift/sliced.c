#include <stdint.h>
#include <stddef.h>

#include "sliced.h"

static const int round_constant[] = {
        // rounds 0-15
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F, 0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E,
        // rounds 16-31
        0x1D, 0x3A, 0x35, 0x2B, 0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E, 0x1C, 0x38,
        // rounds 32-47
        0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36, 0x2D, 0x1A, 0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04
};

static void print_unpacked(const uint64_t m[]);

void swapmove(uint64_t *a, uint64_t *b, uint64_t m, int n)
{
        uint64_t t = ((*a >> n) ^ *b) & m;
        *b ^= t;
        *a ^= (t << n);
}

// this operation is its own inverse
// m[0] = a0 b0 c0 d0 e0 f0 g0 h0 a8 b8 c8 d8 e8 f8 g8 h8 ...
// m[1] = a1 b1 c1 d1 e1 f1 g1 h1 a9 b9 c9 d9 e9 f9 g9 h9 ...
// ...
void bits_pack(uint64_t m[8])
{
        // take care not to shift mask bits out of the register
        swapmove(&m[0], &m[1], 0x5555555555555555UL, 1);
        swapmove(&m[2], &m[3], 0x5555555555555555UL, 1);
        swapmove(&m[4], &m[5], 0x5555555555555555UL, 1);
        swapmove(&m[6], &m[7], 0x5555555555555555UL, 1);

        swapmove(&m[0], &m[2], 0x3333333333333333UL, 2);
        swapmove(&m[1], &m[3], 0x3333333333333333UL, 2);
        swapmove(&m[4], &m[6], 0x3333333333333333UL, 2);
        swapmove(&m[5], &m[7], 0x3333333333333333UL, 2);

        // make bytes (a0 b0 c0 d0 a4 b4 c4 d4 -> a0 b0 c0 d0 e0 f0 g0 h0)
        swapmove(&m[0], &m[4], 0x0f0f0f0f0f0f0f0fUL, 4);
        swapmove(&m[2], &m[6], 0x0f0f0f0f0f0f0f0fUL, 4);
        swapmove(&m[1], &m[5], 0x0f0f0f0f0f0f0f0fUL, 4);
        swapmove(&m[3], &m[7], 0x0f0f0f0f0f0f0f0fUL, 4);
}

void print_unpacked(const uint64_t m[])
{
        uint64_t m1[8];
        memcpy(m1, m, 8 * sizeof(m[0]));
        bits_pack(m1);
        for (size_t i = 0; i < 8; i++) {
                printf("%016lx ", m1[i]);
        }

        printf("\n");
}

void gift_64_sliced_generate_round_keys(uint64_t round_keys[ROUNDS_GIFT_SLICED_64][8],
                                        const uint64_t key[2])
{
        uint64_t key_state[] = {key[0], key[1]};
        for (int round = 0; round < ROUNDS_GIFT_SLICED_64; round++) {
                uint32_t v = (key_state[0] >> 0 ) & 0xffff;
                uint32_t u = (key_state[0] >> 16) & 0xffff;

                // add round key (RK=U||V)
                round_keys[round][0] = 0;
                for (size_t i = 0; i < 16; i++) {
                        int key_bit_v   = (v >> i)  & 0x1;
                        int key_bit_u   = (u >> i)  & 0x1;
                        round_keys[round][0] ^= (uint64_t)key_bit_v << (i * 4);
                        round_keys[round][0] ^= (uint64_t)key_bit_u << (i * 4 + 1);
                }

                // add single bit
                round_keys[round][0] ^= (1UL << 63);

                // add round constants
                round_keys[round][0] ^= (uint64_t)((round_constant[round] >> 0) & 0x1) << 3;
                round_keys[round][0] ^= (uint64_t)((round_constant[round] >> 1) & 0x1) << 7;
                round_keys[round][0] ^= (uint64_t)((round_constant[round] >> 2) & 0x1) << 11;
                round_keys[round][0] ^= (uint64_t)((round_constant[round] >> 3) & 0x1) << 15;
                round_keys[round][0] ^= (uint64_t)((round_constant[round] >> 4) & 0x1) << 19;
                round_keys[round][0] ^= (uint64_t)((round_constant[round] >> 5) & 0x1) << 23;

                // copy and pack bits
                round_keys[round][1] = round_keys[round][0];
                round_keys[round][2] = round_keys[round][0];
                round_keys[round][3] = round_keys[round][0];
                round_keys[round][4] = round_keys[round][0];
                round_keys[round][5] = round_keys[round][0];
                round_keys[round][6] = round_keys[round][0];
                round_keys[round][7] = round_keys[round][0];
                bits_pack(round_keys[round]);

                // update key state
                int k0 = (key_state[0] >> 0 ) & 0xffffUL;
                int k1 = (key_state[0] >> 16) & 0xffffUL;
                k0 = (k0 >> 12) | ((k0 & 0xfff) << 4);
                k1 = (k1 >> 2 ) | ((k1 & 0x3  ) << 14);
                key_state[0] >>= 32;
                key_state[0] |= (key_state[1] & 0xffffffffUL) << 32;
                key_state[1] >>= 32;
                key_state[1] |= ((uint64_t)k0 << 32) | ((uint64_t)k1 << 48);
        }
}

void gift_64_sliced_subcells(uint64_t s[8])
{
        for (size_t i = 0; i < 2; i++) {
                size_t j = i * 4;
                s[j + 1]    ^= s[j + 0] & s[j + 2];
                uint64_t t  =  s[j + 0] ^ (s[j + 1] & s[j + 3]);
                s[j + 2]    ^= t        | s[j + 1];
                s[j + 0]    =  s[j + 3] ^ s[j + 2];
                s[j + 1]    ^= s[j + 0];
                s[j + 0]    = ~s[j + 0];
                s[j + 2]    ^= t        & s[j + 1];
                s[j + 3]    =  t;
        }
}


void gift_64_sliced_subcells_inv(uint64_t s[8])
{
        for (size_t i = 0; i < 2; i++) {
                size_t j = i * 4;
                uint64_t t  = s[j + 3];
                s[j + 2]    ^= t        & s[j + 1];
                s[j + 0]    = ~s[j + 0];
                s[j + 1]    ^= s[j + 0];
                s[j + 3]    =  s[j + 0] ^ s[j + 2];
                s[j + 2]    ^= t        | s[j + 1];
                s[j + 0]    = t         ^ (s[j + 1] & s[j + 3]);
                s[j + 1]    ^= s[j + 0] & s[j + 2];
        }
}

void gift_64_sliced_permute(uint64_t s[8])
{
        uint8_t src[8][8];
        memcpy(src, s, sizeof(src));
        uint8_t *dst = (uint8_t*)s;

        dst[0]  = src[0][0]; dst[10] = src[1][0]; dst[20] = src[2][0];
        dst[30] = src[3][0]; dst[6]  = src[4][0]; dst[8]  = src[5][0];
        dst[18] = src[6][0]; dst[28] = src[7][0]; dst[4]  = src[0][1];
        dst[14] = src[1][1]; dst[16] = src[2][1]; dst[26] = src[3][1];
        dst[2]  = src[4][1]; dst[12] = src[5][1]; dst[22] = src[6][1];
        dst[24] = src[7][1]; dst[32] = src[0][2]; dst[42] = src[1][2];
        dst[52] = src[2][2]; dst[62] = src[3][2]; dst[38] = src[4][2];
        dst[40] = src[5][2]; dst[50] = src[6][2]; dst[60] = src[7][2];
        dst[36] = src[0][3]; dst[46] = src[1][3]; dst[48] = src[2][3];
        dst[58] = src[3][3]; dst[34] = src[4][3]; dst[44] = src[5][3];
        dst[54] = src[6][3]; dst[56] = src[7][3]; dst[1]  = src[0][4];
        dst[11] = src[1][4]; dst[21] = src[2][4]; dst[31] = src[3][4];
        dst[7]  = src[4][4]; dst[9]  = src[5][4]; dst[19] = src[6][4];
        dst[29] = src[7][4]; dst[5]  = src[0][5]; dst[15] = src[1][5];
        dst[17] = src[2][5]; dst[27] = src[3][5]; dst[3]  = src[4][5];
        dst[13] = src[5][5]; dst[23] = src[6][5]; dst[25] = src[7][5];
        dst[33] = src[0][6]; dst[43] = src[1][6]; dst[53] = src[2][6];
        dst[63] = src[3][6]; dst[39] = src[4][6]; dst[41] = src[5][6];
        dst[51] = src[6][6]; dst[61] = src[7][6]; dst[37] = src[0][7];
        dst[47] = src[1][7]; dst[49] = src[2][7]; dst[59] = src[3][7];
        dst[35] = src[4][7]; dst[45] = src[5][7]; dst[55] = src[6][7];
        dst[57] = src[7][7];
}

void gift_64_sliced_permute_inv(uint64_t s[8])
{
        uint8_t src[8][8];
        memcpy(src, s, sizeof(src));
        uint8_t *dst = (uint8_t*)s;

        dst[0]  = src[0][0]; dst[8]  = src[1][2]; dst[16] = src[2][4];
        dst[24] = src[3][6]; dst[32] = src[0][6]; dst[40] = src[1][0];
        dst[48] = src[2][2]; dst[56] = src[3][4]; dst[1]  = src[0][4];
        dst[9]  = src[1][6]; dst[17] = src[2][0]; dst[25] = src[3][2];
        dst[33] = src[0][2]; dst[41] = src[1][4]; dst[49] = src[2][6];
        dst[57] = src[3][0]; dst[2]  = src[4][0]; dst[10] = src[5][2];
        dst[18] = src[6][4]; dst[26] = src[7][6]; dst[34] = src[4][6];
        dst[42] = src[5][0]; dst[50] = src[6][2]; dst[58] = src[7][4];
        dst[3]  = src[4][4]; dst[11] = src[5][6]; dst[19] = src[6][0];
        dst[27] = src[7][2]; dst[35] = src[4][2]; dst[43] = src[5][4];
        dst[51] = src[6][6]; dst[59] = src[7][0]; dst[4]  = src[0][1];
        dst[12] = src[1][3]; dst[20] = src[2][5]; dst[28] = src[3][7];
        dst[36] = src[0][7]; dst[44] = src[1][1]; dst[52] = src[2][3];
        dst[60] = src[3][5]; dst[5]  = src[0][5]; dst[13] = src[1][7];
        dst[21] = src[2][1]; dst[29] = src[3][3]; dst[37] = src[0][3];
        dst[45] = src[1][5]; dst[53] = src[2][7]; dst[61] = src[3][1];
        dst[6]  = src[4][1]; dst[14] = src[5][3]; dst[22] = src[6][5];
        dst[30] = src[7][7]; dst[38] = src[4][7]; dst[46] = src[5][1];
        dst[54] = src[6][3]; dst[62] = src[7][5]; dst[7]  = src[4][5];
        dst[15] = src[5][7]; dst[23] = src[6][1]; dst[31] = src[7][3];
        dst[39] = src[4][3]; dst[47] = src[5][5]; dst[55] = src[6][7];
        dst[63] = src[7][1];
}

void gift_64_sliced_encrypt(uint64_t c[8], const uint64_t m[8], const uint64_t key[2])
{
        // generate round keys
        uint64_t round_keys[ROUNDS_GIFT_SLICED_64][8];
        gift_64_sliced_generate_round_keys(round_keys, key);

        // copy to state (eight 64-bit "registers") and pack message bits
        uint64_t state[8];
        memcpy(state, m, 8 * sizeof(m[0]));
        bits_pack(state);

        // round loop
        for (int round = 0; round < ROUNDS_GIFT_SLICED_64; round++) {
                gift_64_sliced_subcells(state);
#ifdef DEBUG
                printf("GIFT_64_SLICED_ENCRYPT round %02d, subcells:      ", round);
                print_unpacked(state);
#endif
                gift_64_sliced_permute(state);
#ifdef DEBUG
                printf("GIFT_64_SLICED_ENCRYPT round %02d, permbits:      ", round);
                print_unpacked(state);
#endif
                for (size_t j = 0; j < 8; j++) {
                        state[j] ^= round_keys[round][j];
                }
#ifdef DEBUG
                printf("GIFT_64_SLICED_ENCRYPT round %02d, add round key: ", round);
                print_unpacked(state);
#endif
        }

        // unpack ciphertext bits and copy to ciphertext buffer
        bits_pack(state);
        memcpy(c, state, 8 * sizeof(state[0]));
}

void gift_64_sliced_decrypt(uint64_t m[8], const uint64_t c[8], const uint64_t key[2])
{
        // generate round keys
        uint64_t round_keys[ROUNDS_GIFT_SLICED_64][8];
        gift_64_sliced_generate_round_keys(round_keys, key);

        // copy to state (eight 64-bit "registers") and pack message bits
        uint64_t state[8];
        memcpy(state, c, 8 * sizeof(m[0]));
        bits_pack(state);

        // round loop
        for (int round = ROUNDS_GIFT_SLICED_64 - 1; round >= 0; round--) {
                for (size_t j = 0; j < 8; j++) {
                        state[j] ^= round_keys[round][j];
                }
#ifdef DEBUG
                printf("GIFT_64_SLICED_DECRYPT round %02d, add round key: ", round);
                print_unpacked(state);
#endif
                gift_64_sliced_permute_inv(state);

#ifdef DEBUG
                printf("GIFT_64_SLICED_DECRYPT round %02d, permbits inv:  ", round);
                print_unpacked(state);
#endif
                gift_64_sliced_subcells_inv(state);
#ifdef DEBUG
                printf("GIFT_64_SLICED_DECRYPT round %02d, subcells inv:  ", round);
                print_unpacked(state);
#endif
        }

        // unpack message bits and copy to message buffer
        bits_pack(state);
        memcpy(m, state, 8 * sizeof(state[0]));
}
