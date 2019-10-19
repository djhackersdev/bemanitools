#define LOG_MODULE "crypto"

#include "util/crypto.h"
#include "util/log.h"
#include "util/mem.h"

#include <stdlib.h>
#include <string.h>

static const char vista_prov[] =
    "Microsoft Enhanced RSA and AES Cryptographic Provider";
static const char winxp_prov[] =
    "Microsoft Enhanced RSA and AES Cryptographic Provider (Prototype)";

static int init_count;
static HCRYPTPROV prov;

void crypto_init(void)
{
    const char *prov_name;
    OSVERSIONINFOEX osix;

    if (init_count++ != 0) {
        return;
    }

    log_misc("Initializing win32 crypto wrappers");

    memset(&osix, 0, sizeof(osix));
    osix.dwOSVersionInfoSize = sizeof(osix);
    GetVersionEx((void *) &osix);

    if (osix.dwMajorVersion < 6) {
        prov_name = winxp_prov;
    } else {
        prov_name = vista_prov;
    }

    if (!CryptAcquireContext(
            &prov, NULL, prov_name, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        log_fatal(
            "CryptAcquireContext failed: %08x", (unsigned int) GetLastError());
    }
}

void crypto_gen_random(void *bytes, size_t nbytes)
{
    CryptGenRandom(prov, nbytes, bytes);
}

void crypto_fini(void)
{
    if (--init_count != 0) {
        return;
    }

    log_misc("Shutting down win32 crypto wrappers");

    if (!CryptReleaseContext(prov, 0)) {
        log_fatal(
            "CryptReleaseContext failed: %08x", (unsigned int) GetLastError());
    }
}

void md5_compute(
    const void *in_bytes,
    size_t in_nbytes,
    uint8_t *out_bytes,
    size_t out_nbytes)
{
    struct md5 md5;

    md5_init(&md5);
    md5_append(&md5, in_bytes, in_nbytes);
    md5_fini(&md5, out_bytes, out_nbytes);
}

void md5_init(struct md5 *md5)
{
    if (!CryptCreateHash(prov, CALG_MD5, 0, 0, &md5->hash)) {
        log_fatal(
            "CryptCreateHash(CALG_MD5) failed: %08x",
            (unsigned int) GetLastError());
    }
}

void md5_append(struct md5 *md5, const void *bytes, size_t nbytes)
{
    if (!CryptHashData(md5->hash, bytes, nbytes, 0)) {
        log_fatal("CryptHashData failed: %08x", (unsigned int) GetLastError());
    }
}

void md5_fini(struct md5 *md5, uint8_t *output, size_t nbytes)
{
    DWORD tmp;

    log_assert(output != NULL && nbytes == 16);

    tmp = nbytes;

    if (output != NULL) {
        if (!CryptGetHashParam(md5->hash, HP_HASHVAL, output, &tmp, 0)) {
            log_fatal(
                "Error getting MD5 result: %08x",
                (unsigned int) GetLastError());
        }
    }

    if (!CryptDestroyHash(md5->hash)) {
        log_fatal(
            "CryptDestroyHash failed: %08x", (unsigned int) GetLastError());
    }
}

#define SWAP(x, y, tmp) \
    tmp = x;            \
    x = y;              \
    y = tmp

void arc4_init(struct arc4 *arc4, const uint8_t *key, size_t key_nbytes)
{
    size_t i;
    uint8_t j;
    uint8_t tmp;

    arc4->i = 0;
    arc4->j = 0;

    for (i = 0; i < 256; i++) {
        arc4->S[i] = (uint8_t) i;
    }

    j = 0;

    for (i = 0; i < 256; i++) {
        j = j + arc4->S[i] + key[i % key_nbytes];
        SWAP(arc4->S[i], arc4->S[j], tmp);
    }
}

void arc4_apply(struct arc4 *arc4, uint8_t *bytes, size_t nbytes)
{
    size_t i;
    uint8_t tmp;

    for (i = 0; i < nbytes; i++) {
        arc4->i += 1;
        arc4->j += arc4->S[arc4->i];

        SWAP(arc4->S[arc4->i], arc4->S[arc4->j], tmp);

        bytes[i] ^= arc4->S[(uint8_t)(arc4->S[arc4->i] + arc4->S[arc4->j])];
    }
}

static uint32_t blowfish_F(struct blowfish *blowfish, uint32_t x)
{
    unsigned short a, b, c, d;
    uint32_t y;

    d = x & 0x00FF;
    x >>= 8;
    c = x & 0x00FF;
    x >>= 8;
    b = x & 0x00FF;
    x >>= 8;
    a = x & 0x00FF;

    y = blowfish->S[0][a] + blowfish->S[1][b];
    y = y ^ blowfish->S[2][c];
    y = y + blowfish->S[3][d];
    return y;
}

void blowfish_init(
    struct blowfish *blowfish, const uint8_t *key, size_t key_length)
{
    int i, j, k;
    uint32_t data, datal, datar;

    j = 0;
    for (i = 0; i < 16 + 2; ++i) {
        data = 0x00000000;

        for (k = 0; k < 4; ++k) {
            data = (data << 8) | key[j];
            j = j + 1;
            if (j >= key_length)
                j = 0;
        }

        blowfish->P[i] = blowfish->P[i] ^ data;
    }

    datal = 0x00000000;
    datar = 0x00000000;

    for (i = 0; i < 16 + 2; i += 2) {
        blowfish_encrypt(blowfish, &datal, &datar);
        blowfish->P[i] = datal;
        blowfish->P[i + 1] = datar;
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 256; j += 2) {
            blowfish_encrypt(blowfish, &datal, &datar);
            blowfish->S[i][j] = datal;
            blowfish->S[i][j + 1] = datar;
        }
    }
}

void blowfish_encrypt(struct blowfish *blowfish, uint32_t *xl, uint32_t *xr)
{
    uint32_t Xl;
    uint32_t Xr;
    uint32_t temp;
    short i;

    Xl = *xl;
    Xr = *xr;

    for (i = 0; i < 16; ++i) {
        Xl = Xl ^ blowfish->P[i];
        Xr = blowfish_F(blowfish, Xl) ^ Xr;
        temp = Xl;
        Xl = Xr;
        Xr = temp;
    }

    temp = Xl;
    Xl = Xr;
    Xr = temp;
    Xr = Xr ^ blowfish->P[16];
    Xl = Xl ^ blowfish->P[16 + 1];
    *xl = Xl;
    *xr = Xr;
}

void blowfish_decrypt(struct blowfish *blowfish, uint32_t *xl, uint32_t *xr)
{
    uint32_t Xl;
    uint32_t Xr;
    uint32_t temp;
    short i;

    Xl = *xl;
    Xr = *xr;

    for (i = 16 + 1; i > 1; --i) {
        Xl = Xl ^ blowfish->P[i];
        Xr = blowfish_F(blowfish, Xl) ^ Xr;
        /* Exchange Xl and Xr */
        temp = Xl;
        Xl = Xr;
        Xr = temp;
    }

    /* Exchange Xl and Xr */
    temp = Xl;
    Xl = Xr;
    Xr = temp;
    Xr = Xr ^ blowfish->P[1];
    Xl = Xl ^ blowfish->P[0];
    *xl = Xl;
    *xr = Xr;
}
