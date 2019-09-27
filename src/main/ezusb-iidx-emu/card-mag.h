#ifndef EZUSB_IIDX_EMU_CARD_MAG_H
#define EZUSB_IIDX_EMU_CARD_MAG_H

#include <stdint.h>
#include <stdbool.h>

#define MAG_CARD_NUM_DATA_SECTORS 5

/* Structure for the data format of a magnetic card */
struct ezusb_iidx_emu_card_mag_data {
    struct {
        /* several flags (used, fixed flags) */
        uint8_t flags;
        /* card version:
         * C02
         * D01
         * E11
         * ECO
         * @@@ -> eamuse common
         * ??? -> konami common
         */
        uint8_t card_version[3];
        /* crc8 header only */
        uint8_t checksum;
    } header;

    union {
        /* special treatment for 9th, only */
        struct {
            /* valid card types 0 to 4 */
            uint8_t card_type;
            /* 64-bit card id */
            uint8_t card_id[8];
            /* crc8 for id and type*/
            uint8_t checksum;
        } data_sector_9th[MAG_CARD_NUM_DATA_SECTORS];

        /* 10th, Red, HappySky */
        struct {
            /* 64-bit card id */
            uint8_t card_id[8];
            /* valid card types 0 to 4 */
            uint8_t card_type;
            /* crc8 for id and type*/
            uint8_t checksum;
        } data_sector[MAG_CARD_NUM_DATA_SECTORS];
    };

    /* part of the data, but not used */
    uint8_t padding[8];
    /* crc16 for whole payload (i.e. everything minus header) */
    /* uint8_t[2] array to avoid alignment issues */
    uint8_t checksum[2];
};

/**
 * Generate a data blob resembling the structure of the data stored on real
 * magnetic cards
 *
 * @param card Pointer to reserved memory to write the resulting card data to
 * @param card_id 64-bit card id to use for the magnetic card
 * @param card_type Type of the magnetic card (valid: 0 - 4)
 * @param card_used True to flag the card used (recommended), false unused
 * @param card_version Version branding for the card (mcode of the game).
 * @see struct magnetic_card
 */
void ezusb_iidx_emu_card_mag_generate_data(
        struct ezusb_iidx_emu_card_mag_data* card, uint8_t* card_id,
        uint8_t card_type, bool card_used, const char* card_version);

#endif
