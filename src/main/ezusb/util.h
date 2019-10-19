/**
 * Utility module for ezusb1 (legacy) and ezusb2 (FX2) device handling.
 *
 * @author icex2
 */
#ifndef EZUSB_UTIL_H
#define EZUSB_UTIL_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Struct with identifier information of the board.
 */
struct ezusb_ident {
    char name[256];
    uint16_t pid;
    uint16_t vid;
};

/**
 * A single segment of the firmware.
 */
struct ezusb_firmware_segment {
    uint16_t size;
    uint16_t offset;
    uint8_t data[];
};

/**
 * Ezusb firmware struct. Holds the firmware separated into segments.
 */
struct ezusb_firmware {
    uint16_t crc;
    uint16_t segment_count;
    struct ezusb_firmware_segment **segments;
};

/**
 * Load a firmware binary file into memory.
 *
 * @oaram file Path to firmware binary file.
 * @return Valid firmware structure on success, NULL on failure.
 */
struct ezusb_firmware *ezusb_firmware_load(const char *file);

/**
 * Save the in-memory firmware to a file.
 *
 * @param file Target file to save to.
 * @param fw Valid firmware structure to store to disk.
 * @return True on success, false on error.
 */
bool ezusb_firmware_save(const char *file, struct ezusb_firmware *fw);

/**
 * Allocate and initialize a new (empty = no segments) ezusb firmware structure.
 *
 * @return Allocated and initialized structure.
 */
struct ezusb_firmware *ezusb_firmware_alloc();

/**
 * Allocate and initialize a new firmware segment.
 *
 * @param offset Offset of the segment.
 * @param size Size of the segment
 * @param data Pointer to a buffer with firmware data of at least the
 *        specified size.
 * @return Allocated and initialized structure.
 */
struct ezusb_firmware_segment *
ezusb_firmware_segment_alloc(uint16_t offset, uint16_t size, void *data);

/**
 * Add a segment to a firmware structure.
 *
 * @param fw Valid firmware structure to add the segment to.
 * @param segment Valid segment to add to the end of the segment list of the
 *        firmware.
 */
void ezusb_firmware_add_segment(
    struct ezusb_firmware *fw, struct ezusb_firmware_segment *segment);

/**
 * Calculate the checksum of the provided firmware.
 *
 * @param fw Firmware to checksum.
 * @return CRC16 of firmware.
 */
uint16_t ezusb_firmware_crc(struct ezusb_firmware *fw);

/**
 * Free a previously allocated firmware structure (also cleans up all
 * referenced segments).
 *
 * @param fw Firmware struct to free.
 */
void ezusb_firmware_free(struct ezusb_firmware *fw);

#endif