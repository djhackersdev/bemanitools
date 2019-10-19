#ifndef SECURITY_UTIL_H
#define SECURITY_UTIL_H

#include <stdint.h>

/**
 * Pack 8 bytes of input data into 6 bytes payload output data. This is used to
 * pack/encode mcodes from roundplug dongles, e.g. GCH44JAA.
 *
 * @param in Input data of 8 bytes length, e.g. mcode.
 * @param out Target buffer for encoded data with at least 6 bytes capacity.
 */
void security_util_8_to_6_encode(const uint8_t *in, uint8_t *out);

/**
 * Decode/unpack 6 bytes of encoded roundplug data, e.g. encoded mcode, to
 * the full 8 bytes width.
 *
 * @param in Input encoded payload of 6 bytes length.
 * @param out Target buffer for decoded data with at least 8 bytes capacity.
 */
void security_util_6_to_8_decode(const uint8_t *in, uint8_t *out);

/**
 * Same as security_util_8_to_6_encode but in reversed byte order.
 *
 * @param in Input data of 8 bytes length, e.g. mcode.
 * @param out Target buffer for encoded data with at least 6 bytes capacity.
 */
void security_util_8_to_6_encode_reverse(const uint8_t *in, uint8_t *out);

/**
 * Same as security_util_6_to_8_decode but in reversed byte order.
 *
 * @param in Input encoded payload of 6 bytes length.
 * @param out Target buffer for decoded data with at least 8 bytes capacity.
 */
void security_util_6_to_8_decode_reverse(const uint8_t *in, uint8_t *out);

#endif
