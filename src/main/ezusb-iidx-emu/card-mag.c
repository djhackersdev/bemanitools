#include "ezusb-iidx-emu/card-mag.h"

#include <string.h>

#include "security/mcode.h"

#include "util/crc.h"
#include "util/log.h"

static const uint16_t ezusb_iidx_emu_card_mag_checksum_table_payload[256] = {
    0x0, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF, 0x8C48, 0x9DC1, 0x0AF5A,
    0x0BED3, 0x0CA6C, 0x0DBE5, 0x0E97E, 0x0F8F7, 0x1081, 0x108, 0x3393, 0x221A, 0x56A5,
    0x472C, 0x75B7, 0x643E, 0x9CC9, 0x8D40, 0x0BFDB, 0x0AE52, 0x0DAED, 0x0CB64, 0x0F9FF,
    0x0E876, 0x2102, 0x308B, 0x210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD, 0x0AD4A,
    0x0BCC3, 0x8E58, 0x9FD1, 0x0EB6E, 0x0FAE7, 0x0C87C, 0x0D9F5, 0x3183, 0x200A,
    0x1291, 0x318, 0x77A7, 0x662E, 0x54B5, 0x453C, 0x0BDCB, 0x0AC42, 0x9ED9, 0x8F50,
    0x0FBEF, 0x0EA66, 0x0D8FD, 0x0C974, 0x4204, 0x538D, 0x6116, 0x709F, 0x420, 0x15A9,
    0x2732, 0x36BB, 0x0CE4C, 0x0DFC5, 0x0ED5E, 0x0FCD7, 0x8868, 0x99E1, 0x0AB7A,
    0x0BAF3, 0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x528, 0x37B3, 0x263A, 0x0DECD,
    0x0CF44, 0x0FDDF, 0x0EC56, 0x98E9, 0x8960, 0x0BBFB, 0x0AA72, 0x6306, 0x728F,
    0x4014, 0x519D, 0x2522, 0x34AB, 0x630, 0x17B9, 0x0EF4E, 0x0FEC7, 0x0CC5C, 0x0DDD5,
    0x0A96A, 0x0B8E3, 0x8A78, 0x9BF1, 0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A,
    0x16B1, 0x738, 0x0FFCF, 0x0EE46, 0x0DCDD, 0x0CD54, 0x0B9EB, 0x0A862, 0x9AF9,
    0x8B70, 0x8408, 0x9581, 0x0A71A, 0x0B693, 0x0C22C, 0x0D3A5, 0x0E13E, 0x0F0B7,
    0x840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF, 0x9489, 0x8500,
    0x0B79B, 0x0A612, 0x0D2AD, 0x0C324, 0x0F1BF, 0x0E036, 0x18C1, 0x948, 0x3BD3,
    0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E, 0x0A50A, 0x0B483, 0x8618, 0x9791, 0x0E32E,
    0x0F2A7, 0x0C03C, 0x0D1B5, 0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74,
    0x5DFD, 0x0B58B, 0x0A402, 0x9699, 0x8710, 0x0F3AF, 0x0E226, 0x0D0BD, 0x0C134,
    0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C, 0x0C60C, 0x0D785,
    0x0E51E, 0x0F497, 0x8028, 0x91A1, 0x0A33A, 0x0B2B3, 0x4A44, 0x5BCD, 0x6956, 0x78DF,
    0x0C60, 0x1DE9, 0x2F72, 0x3EFB, 0x0D68D, 0x0C704, 0x0F59F, 0x0E416, 0x90A9, 0x8120,
    0x0B3BB, 0x0A232, 0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
    0x0E70E, 0x0F687, 0x0C41C, 0x0D595, 0x0A12A, 0x0B0A3, 0x8238, 0x93B1, 0x6B46,
    0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9, 0x0F78F, 0x0E606, 0x0D49D,
    0x0C514, 0x0B1AB, 0x0A022, 0x92B9, 0x8330, 0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3,
    0x2C6A, 0x1EF1, 0x0F78
};

/* ------------------------------------------------------------------ */

// static void target_form_8_digit_to_long(const uint8_t* in_buf, uint8_t* out_buf)
// {
//     out_buf[0] = (in_buf[0] & 0x3F) + 0x20;
//     out_buf[1] = ((in_buf[1] >> 2) & 0x3F) + 0x20;
//     out_buf[2] = (((in_buf[1] & 0x03) << 4) | ((in_buf[2] >> 4) & 0x0F)) + 0x20;
// }

static void ezusb_iidx_emu_card_mag_target_form_8_digit_to_short(
        const uint8_t* in_buf, uint8_t* out_buf)
{
    out_buf[0] = (out_buf[0] & 0xC0) | ((in_buf[0] - 0x20) & 0x3F);
    out_buf[1] = (((in_buf[1] - 0x20) & 0x3F) << 2) |
                 (((in_buf[2] - 0x20) >> 4) & 0x03);
    out_buf[2] = (((in_buf[2] - 0x20) & 0x0F) << 4) | (out_buf[2] & 0x0F);
}

static uint16_t ezusb_iidx_emu_card_mag_calc_checksum_payload(
        const uint8_t* buffer, size_t length)
{
    unsigned int result; // eax@1
    int v3; // esi@2
    const uint8_t* v4; // ecx@2

    result = 0xFFFF;

    v3 = length;
    v4 = buffer;

    do {
        result = ezusb_iidx_emu_card_mag_checksum_table_payload[*v4++ ^
        (unsigned __int8) result] ^ (result >> 8);
        --v3;
    } while (v3);

    return result;
}

static void ezusb_iidx_emu_card_mag_update_checksums(
        struct ezusb_iidx_emu_card_mag_data* card, const char* card_version)
{
    /* at least the header checksum and the checksums for the
       data sectors are the same for all games...*/

    /* checksum the header only */
    card->header.checksum = crc8(&card->header.flags, 4, 0);

    /* checksum the actual payload of the payload */
    for (uint8_t i = 0; i < MAG_CARD_NUM_DATA_SECTORS; i++) {
        /* seed is hardcoded to 0 */
        card->data_sector[i].checksum = crc8(
                (uint8_t*) &card->data_sector[i], 9, 0);
    }

    /* but the checksum for all data sectors differs... */
    if (    !memcmp(card_version, SECURITY_MCODE_GAME_IIDX_9, 
                SECURITY_MCODE_GAME_LEN) ||
            !memcmp(card_version, SECURITY_MCODE_GAME_IIDX_11, 
                SECURITY_MCODE_GAME_LEN) ||
            !memcmp(card_version, SECURITY_MCODE_GAME_IIDX_12, 
                SECURITY_MCODE_GAME_LEN) ) {
        /* checksum the whole payload with the unused part (padding)
           big endian or mistake by konami dev... */
        uint16_t crc = crc16((const uint8_t*) &card->data_sector[0],
                MAG_CARD_NUM_DATA_SECTORS * 10 + sizeof(card->padding), 0);
        uint16_t* ptr = (uint16_t*) card->checksum;
        *ptr = crc;
    } else {
        /* 10th style */
        uint16_t* ptr = (uint16_t*) card->checksum;
        *ptr = ezusb_iidx_emu_card_mag_calc_checksum_payload(
            (const uint8_t*) &card->data_sector[0],
            MAG_CARD_NUM_DATA_SECTORS * 10 + sizeof(card->padding));
    }
}

void ezusb_iidx_emu_card_mag_generate_data(
        struct ezusb_iidx_emu_card_mag_data* card, uint8_t* card_id,
        uint8_t card_type, bool card_used, const char* card_version)
{
    memset(card, 0, sizeof(struct ezusb_iidx_emu_card_mag_data));

    /* these flags have to be set like this, otherwise
       the card is detected unknown */
    /* 0 on bit 0 */
    card->header.flags &= ~(1 << 0);
    /* 0 on bit 2 */
    card->header.flags &= ~(1 << 2);
    /* 1 on bit 3 */
    card->header.flags |= (1 << 3);
    /* 0 on bit 4 */
    card->header.flags &= ~(1 << 4);
    /* furthermore, the last two bits of the version field are also flags */
    /* 0 on bit 6 */
    card->header.card_version[0] &= ~(1 << 6);
    /* 0 on bit 7 */
    card->header.card_version[0] &= ~(1 << 7);
    /* and the last 4 bits not used by the version in version field[2]
       have to be set to 0001, otherwise the card is of unknown type */
    card->header.card_version[2] = 1;

    ezusb_iidx_emu_card_mag_target_form_8_digit_to_short(
            (const uint8_t*) card_version, card->header.card_version);

    if (card_used) {
        /* the first one is checked if the card is already formated
           for a specific game version */
        card->header.flags |= (1 << 6);
        /* the second flag is used if the card is not formated for
           a specific game version, but a generic eamuse card
           don't use this, because it will interfere on 9th style on the card
           check */
        //card->header.flags |= (1 << 7);
    } else {
        card->header.flags &= ~(1 << 6);
        card->header.flags &= ~(1 << 7);
    }

    /* the game flips this back to little endian
       so we have to give it big endian ordering here */
    for (uint8_t i = 0; i < MAG_CARD_NUM_DATA_SECTORS; i++) {

        if (!memcmp(card_version, SECURITY_MCODE_GAME_IIDX_9, 
                SECURITY_MCODE_GAME_LEN)) {
            memcpy(card->data_sector_9th[i].card_id, card_id, 8);
            card->data_sector_9th[i].card_type = card_type;
        } else {
            memcpy(card->data_sector[i].card_id, card_id, 8);
            card->data_sector[i].card_type = card_type;
        }

    }

    ezusb_iidx_emu_card_mag_update_checksums(card, card_version);
}
