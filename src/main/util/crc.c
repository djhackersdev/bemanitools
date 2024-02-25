#include <stddef.h>
#include <stdint.h>

#include "core/log.h"

uint8_t crc8(const void *ptr, size_t nbytes, uint8_t in)
{
    const uint8_t *bytes;
    uint8_t crc;
    size_t i;
    size_t j;

    log_assert(ptr != NULL);

    bytes = ptr;
    crc = ~in;

    for (i = 0; i < nbytes; i++) {
        crc ^= bytes[i];

        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0x8C;
            } else {
                crc = crc >> 1;
            }
        }
    }

    return ~crc;
}

uint16_t crc16(const void *ptr, size_t nbytes, uint16_t in)
{
    const uint8_t *bytes;
    uint16_t crc;
    size_t i;
    size_t j;

    log_assert(ptr != NULL);

    bytes = ptr;
    crc = ~in;

    for (i = 0; i < nbytes; i++) {
        crc ^= bytes[i];

        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0x8408;
            } else {
                crc = crc >> 1;
            }
        }
    }

    return ~crc;
}

uint16_t crc16_msb(const void *ptr, size_t nbytes, uint16_t in)
{
    const uint8_t *bytes;
    uint16_t crc;
    size_t i;
    size_t j;

    log_assert(ptr != NULL);

    bytes = ptr;
    crc = in;

    for (i = 0; i < nbytes; i++) {
        crc ^= bytes[i] << 8;

        for (j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }

    return crc;
}

uint32_t crc32(const void *ptr, size_t nbytes, uint32_t in)
{
    const uint8_t *bytes;
    uint32_t crc;
    size_t i;
    size_t j;

    log_assert(ptr != NULL);

    bytes = ptr;
    crc = ~in;

    for (i = 0; i < nbytes; i++) {
        crc ^= bytes[i];

        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc = (crc >> 1);
            }
        }
    }

    return ~crc;
}
