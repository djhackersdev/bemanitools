#ifndef SECURITY_RP_BLOWFISH_H
#define SECURITY_RP_BLOWFISH_H

#include <stdint.h>
#include <stdlib.h>

#include "util/crypto.h"

/**
 * Initializes a blowfish context for use with a modified version of the
 * algorithm used by the roundplug module.
 *
 * @param ctx Pointer to a context to initialize.
 * @param key Key to use for encryption.
 * @param key_length Length of the key.
 * @param seed A seed value.
 */
void security_rp_blowfish_init(
    struct blowfish *ctx, const uint8_t *key, size_t key_length, uint32_t seed);

/**
 * Encrypt some data with the modified version of blowfish.
 *
 * @param ctx Initialized context to use for the operation.
 * @param input Buffer with input data to encrypt.
 * @param output Pointer to a buffer to write the encrypted output data to.
 * @param length Length of the input data (output buffer must have min size).
 */
int security_rp_blowfish_enc(
    struct blowfish *ctx, const uint8_t *input, uint8_t *output, int length);

#endif