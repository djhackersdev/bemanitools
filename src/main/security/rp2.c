#include "security/rp2.h"
#include "security/util.h"

#include "util/crypto.h"
#include "util/log.h"

static const uint8_t security_rp2_sign_key_base_black[] = {
    0x32, 0x44, 0x58, 0x47, 0x4C, 0x44, 0x41, 0x43
};

static const uint8_t security_rp2_sign_key_base_white[] = {
    0x45, 0x2D, 0x41, 0x4D, 0x55, 0x53, 0x45, 0x33
};

static uint8_t security_rp2_signature_scramble_table[16] = {
    0x0C, 0x02, 0x0F, 0x01, 
    0x07, 0x09, 0x04, 0x0A,
    0x00, 0x0E, 0x03, 0x0D, 
    0x0B, 0x05, 0x08, 0x06
};

static void security_rp2_create_signiture(const uint8_t *plug_id_enc, 
        const uint8_t *sign_key_packed, uint8_t *out)
{
    uint8_t data[14];
    uint8_t md5[16];
    uint8_t buffer[18];

    memcpy(data, plug_id_enc, 8);
    memcpy(data + 8, sign_key_packed, 6);

    crypto_init();
    md5_compute(data, 14, md5, sizeof(md5));
    crypto_fini();

    for (int i = 0; i < 16; i++) {
        buffer[i] = md5[security_rp2_signature_scramble_table[i]];
    }

    buffer[16] = 0xDE;
    buffer[17] = 0xAD;

    for (int i = 0; i < 6; i++) {
        out[i] = buffer[i + 12] ^ buffer[i + 6] ^ buffer[i];
    }
}

void security_rp2_generate_signed_eeprom_data(
        enum security_rp_util_rp_type type,
        const struct security_mcode* boot_version,
        const struct security_mcode* plug_mcode,
        const struct security_id* plug_id, struct security_rp2_eeprom* out)
{
    uint8_t sign_key[8];
    uint8_t plug_id_enc[8];
    char* boot_version_str;

    log_assert(boot_version);
    log_assert(plug_mcode);
    log_assert(plug_id);
    log_assert(out);

    boot_version_str = (char*) boot_version;
 
    /* -------------------------------- */

    switch (type) {
        case SECURITY_RP_UTIL_RP_TYPE_BLACK:
            memcpy(sign_key, security_rp2_sign_key_base_black, 
                sizeof(sign_key));

            sign_key[0] = boot_version_str[0] ^ sign_key[0];
            sign_key[1] ^= boot_version_str[1];
            sign_key[2] = boot_version_str[2] ^ sign_key[2];
            sign_key[3] = boot_version_str[3] ^ sign_key[3];
            sign_key[4] = boot_version_str[4] ^ sign_key[4];
            sign_key[5] ^= boot_version_str[5];
            sign_key[6] = boot_version_str[6] ^ sign_key[6];
            sign_key[7] = boot_version_str[7] ^ sign_key[7];

            break;

        case SECURITY_RP_UTIL_RP_TYPE_WHITE:
            memcpy(sign_key, security_rp2_sign_key_base_white, 
                sizeof(sign_key));
 
            break;

        default:
            log_assert(false);
            break;
    }

    for (uint8_t i = 0; i < sizeof(sign_key); i++) {
        sign_key[i] ^= 0x40;
    }

    security_util_8_to_6_encode(sign_key, sign_key);

    plug_id_enc[0] = plug_id->checksum;
    plug_id_enc[1] = plug_id->id[2];
    plug_id_enc[2] = plug_id->id[3];
    plug_id_enc[3] = plug_id->id[4];
    plug_id_enc[4] = plug_id->id[5];
    plug_id_enc[5] = plug_id->id[6];
    plug_id_enc[6] = plug_id->id[7];
    plug_id_enc[7] = plug_id->id[1];

    security_rp2_create_signiture(plug_id_enc, sign_key, (uint8_t*) out);

    security_util_8_to_6_encode((const uint8_t*) plug_mcode,
        out->packed_payload);
}