#ifndef UTIL_CRYPTO_H
#define UTIL_CRYPTO_H

#include <windows.h>
#include <wincrypt.h>

#include <stddef.h>
#include <stdint.h>

struct arc4 {
    uint8_t S[256];
    uint8_t i;
    uint8_t j;
};

struct md5 {
    HCRYPTHASH hash;
};

struct blowfish {
    uint32_t P[16 + 2];
    uint32_t S[4][256];
};

void crypto_init(void);
void crypto_gen_random(void *bytes, size_t nbytes);
void crypto_fini(void);

void md5_compute(const void *in_bytes, size_t in_nbytes, uint8_t *out_bytes,
        size_t out_nbytes);
void md5_init(struct md5 *md5);
void md5_append(struct md5 *md5, const void *bytes, size_t nbytes);
void md5_fini(struct md5 *md5, uint8_t *output, size_t nbytes);

void arc4_init(struct arc4 *arc4, const uint8_t *key, size_t key_nbytes);
void arc4_apply(struct arc4 *arc4, uint8_t *bytes, size_t nbytes);

void blowfish_init(struct blowfish* blowfish, const uint8_t* key, size_t key_length);
void blowfish_encrypt(struct blowfish* blowfish, uint32_t* xl, uint32_t* xr);
void blowfish_decrypt(struct blowfish* blowfish, uint32_t* xl, uint32_t* xr);

#endif
