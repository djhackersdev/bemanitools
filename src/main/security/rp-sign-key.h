#ifndef SECURITY_RP_SIGN_KEY_H
#define SECURITY_RP_SIGN_KEY_H

#include <stdint.h>

/**
 * Sign keys used to sign roundplug data.
 */
struct security_rp_sign_key {
    char data[8];
};

/**
 * Signing key used to create eeprom signitures for all white eamuse dongles.
 */
extern const struct security_rp_sign_key security_rp_sign_key_white_eamuse;

/**
 * Signing key used to create eeprom signitures for all black dongles used
 * on PS2 games (as far as we are aware of).
 */
extern const struct security_rp_sign_key security_rp_sign_key_black_ps2;

/**
 * Signing key used to create eeprom signitures for all black dongles used
 * on the following games:
 *
 * - GF & DM V4 to V8 (TODO needs verification)
 * - jubeat (1)
 */
extern const struct security_rp_sign_key security_rp_sign_key_black_gfdmv4;

#endif