#include <string.h>

#include "iface-core/log.h"

#include "security/rp-util.h"
#include "security/rp2.h"
#include "security/util.h"

#include "util/crypto.h"

static uint8_t security_rp2_signature_scramble_table[16] = {
    0x0C,
    0x02,
    0x0F,
    0x01,
    0x07,
    0x09,
    0x04,
    0x0A,
    0x00,
    0x0E,
    0x03,
    0x0D,
    0x0B,
    0x05,
    0x08,
    0x06};

void security_rp2_create_signature(
    const uint8_t *plug_id_enc, const uint8_t *sign_key_packed, uint8_t *out)
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

void security_rp2_generate_signed_eeprom_data_core(
    const uint8_t *sign_key,
    const uint8_t *plug_mcode,
    const uint8_t *plug_id,
    uint8_t *signature,
    uint8_t *packed_payload)
{
    uint8_t sign_key_tmp[8];
    security_util_8_to_6_encode(sign_key, sign_key_tmp);
    security_rp2_create_signature(plug_id, sign_key_tmp, signature);
    security_util_8_to_6_encode(plug_mcode, packed_payload);
}

void security_rp2_generate_signed_eeprom_data(
    enum security_rp_util_rp_type type,
    const struct security_rp_sign_key *sign_key,
    const struct security_mcode *plug_mcode,
    const struct security_id *plug_id,
    struct security_rp2_eeprom *out)
{
    uint8_t sign_key_tmp[8];
    uint8_t plug_id_enc[8];
    const uint8_t *plug_mcode_raw = (const uint8_t *) plug_mcode;

    log_assert(sign_key);
    log_assert(plug_mcode);
    log_assert(plug_id);
    log_assert(out);

    for (int i = 0; i < sizeof(sign_key_tmp); i++) {
        sign_key_tmp[i] = sign_key->data[i] ^ 0x40;

        if (type == SECURITY_RP_UTIL_RP_TYPE_BLACK) {
            sign_key_tmp[i] ^= plug_mcode_raw[i];
        }
    }

    plug_id_enc[0] = plug_id->checksum;
    memcpy(&plug_id_enc[1], &plug_id->id[2], 6);
    plug_id_enc[7] = plug_id->id[1];

    security_rp2_generate_signed_eeprom_data_core(
        sign_key_tmp,
        plug_mcode_raw,
        plug_id_enc,
        out->signature,
        out->packed_payload);
}
