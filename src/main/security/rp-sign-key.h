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
 * Signing key used to create eeprom signatures for all white eamuse dongles
 */
extern const struct security_rp_sign_key security_rp_sign_key_white_eamuse;

/**
 * Signing key used to create eeprom signatures for all black dongles used
 * on Python 2 games.
 */
extern const struct security_rp_sign_key security_rp_sign_key_black_python2;

/**
 * Signing key used to create eeprom signatures for all black dongles used
 * on the following games:
 *
 * - GF & DM V4 to V7
 * - jubeat (1)
 */
extern const struct security_rp_sign_key security_rp_sign_key_black_gfdmv4;

/**
 * Signing key used to create eeprom signatures for all black dongles used
 * on the following games:
 *
 * - GF & DM V8, XG to XG3, Gitadora series
 */
extern const struct security_rp_sign_key security_rp_sign_key_black_gfdmv8;

/**
 * Signing key used to create eeprom signatures for all black dongles used
 * on the following games:
 *
 * - DDR X
 */
extern const struct security_rp_sign_key security_rp_sign_key_black_ddrx;

/**
 * Signing key used to create eeprom signatures for all black dongles used
 * on the following games:
 *
 * - DDR X2 and beyond
 */
extern const struct security_rp_sign_key security_rp_sign_key_black_ddrx2;

/**
 * Signing key used to create eeprom signatures for all black dongles used
 * on the following games:
 *
 * - IIDX 14 to 17
 */
extern const struct security_rp_sign_key security_rp_sign_key_black_iidx;

/**
 * Signing key used to create eeprom signatures for all black dongles used
 * on the following games:
 *
 * - pop'n music 15 to 17
 */
extern const struct security_rp_sign_key security_rp_sign_key_black_popn;

#endif