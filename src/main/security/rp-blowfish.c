#include <stdbool.h>

#include "security/rp-blowfish.h"
#include "security/rp-blowfish-table.h"

#include "util/log.h"

static int security_rp_blowfish_enc_sub(int a1)
{
    int result; // eax@1

    result = a1;

    if ( a1 & 7 ) {
        result = a1 - (a1 & 7) + 8;
    }

    return result;
}

void security_rp_blowfish_init(struct blowfish* ctx, const uint8_t* key,
        size_t key_length, uint32_t seed)
{
    log_assert(ctx);
    log_assert(key);

    memcpy(ctx->S, security_rp_blowfish_table_custom_sbox, sizeof(ctx->S));
    memcpy(ctx->P, security_rp_blowfish_table_custom_pbox, sizeof(ctx->P));

    blowfish_init(ctx, &key[seed], 14);
}

int security_rp_blowfish_enc(struct blowfish* ctx, const uint8_t* input,
        uint8_t* output, int length)
{
    int v4; // ebx@1
    int v5; // ebp@1
    int v6; // esi@1
    int result; // eax@1
    unsigned __int8 *v9; // edx@1
    int v10; // ecx@4
    int v11; // eax@11
    signed int v12; // edi@11
    void *v13; // edi@14
    int v14; // eax@14
    int v15; // edx@14
    int v16; // ecx@15
    int v17; // eax@18
    unsigned int v18; // ecx@18
    int v19; // eax@21
    bool v20; // cf@21
    int v22; // [sp+14h] [bp-8h]@1
    int v23; // [sp+18h] [bp-4h]@1
    int a2a; // [sp+20h] [bp+4h]@2
    unsigned __int8 *outputa; // [sp+24h] [bp+8h]@1

    log_assert(ctx);
    log_assert(input);
    log_assert(output);

    v4 = length;
    v5 = (int) output;
    v6 = (int) input;
    v23 = input == output;
    result = security_rp_blowfish_enc_sub(length);
    v9 = 0;
    v22 = result;
    outputa = 0;

    if (result) {
        a2a = v5 + 4;

        while (1) {
            v10 = v4 - 7;

            if (v23) {

                if ((unsigned int) v9 >= v10) {

                    if ( result - v4 > 0 ) {
                        memset((void *)(v6 + v4), 0, result - v4);
                    }

                    blowfish_encrypt(ctx, (uint32_t*) v6, (uint32_t*) v6 + 4);
                    v6 += 8;
                } else {
                     blowfish_encrypt(ctx, (uint32_t *)v6, (uint32_t *)v6 + 1);
                    v6 += 8;
                }

            } else {

                if ((unsigned int) v9 >= v10) {
                    v13 = (void*) v5;
                    v14 = v4 - (int) v9;
                    v15 = 0;
                    if ( v14 <= 0 )
                        goto LABEL_24;
                    v16 = v14;
                    v15 = v14;

                    do {
                        *(uint8_t *)v13 = *(uint8_t *)v6;
                        v13 = (char *) v13 + 1;
                        ++v6;
                        --v16;
                    } while ( v16 );

                    if ( v14 < 8 ) {
LABEL_24:
                        ((uint8_t*)v4)[0] = 8 - v15;
                        ((uint8_t*)v4)[1] = 8 - v15;
                        v17 = v4 << 16;
                        ((uint16_t*)v17)[0] = v4;
                        v18 = (unsigned int)(8 - v15) >> 2;
                        // poor man's version of memset32
                        //memset32(v13, v17, v18);
                        for (size_t i = 0; i < v18; i ++) {
                            *(((uint32_t*)v13) + i) = v17;
                        }

                        memset((char *)v13 + 4 * v18, 8 - v15,
                                (8 - (uint8_t)v15) & 3);
                    }

                    v4 = length;
                } else {
                    v11 = v5;
                    v12 = 8;

                    do {
                        *(uint8_t *)v11 = *(uint8_t *)(v6 - v5 + v11);
                        ++v11;
                        --v12;
                    } while ( v12 );

                }

                blowfish_encrypt(ctx, (uint32_t*) v5, (uint32_t*) a2a);
                v6 += 8;
                v5 += 8;
                a2a += 8;
            }

            v19 = (int)(outputa + 8);
            outputa = (unsigned __int8 *) v19;
            v20 = v19 < (unsigned int) v22;
            result = v22;

            if ( !v20 ) {
                break;
            }

            v9 = outputa;
        }
    }

    return result;
}