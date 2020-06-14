#ifndef UTIL_CRC_H
#define UTIL_CRC_H

#include <stddef.h>
#include <stdint.h>

uint8_t crc8(const void *ptr, size_t nbytes, uint8_t in);
uint16_t crc16(const void *ptr, size_t nbytes, uint16_t in);
uint16_t crc16_msb(const void *ptr, size_t nbytes, uint16_t in);
uint32_t crc32(const void *ptr, size_t nbytes, uint32_t in);

#endif
