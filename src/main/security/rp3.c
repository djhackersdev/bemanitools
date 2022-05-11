#include "security/rp2.h"
#include "security/rp3.h"
#include "security/rp-util.h"
#include "security/util.h"

#include "util/crc.h"
#include "util/crypto.h"
#include "util/log.h"

void security_rp3_generate_signed_eeprom_data(
    enum security_rp_util_rp_type type,
    const struct security_rp_sign_key *sign_key,
    const struct security_mcode *plug_mcode,
    const struct security_id *plug_id,
    struct security_rp3_eeprom *out)
{
    uint8_t sign_key_tmp[8];
    uint8_t plug_id_reversed[8];

    log_assert(sign_key);
    log_assert(plug_mcode);
    log_assert(plug_id);
    log_assert(out);

    memcpy(sign_key_tmp, sign_key, sizeof(sign_key_tmp));

    if (type == SECURITY_RP_UTIL_RP_TYPE_BLACK) {
        for (int i = 0; i < sizeof(sign_key_tmp); i++) {
            sign_key_tmp[i] ^= ((const uint8_t *) plug_mcode)[i];
        }
    }

    security_util_8_to_6_encode(sign_key_tmp, sign_key_tmp);

    for (int i = 0; i < sizeof(plug_id_reversed); i++) {
        plug_id_reversed[i] = plug_id->id[7 - i];
    }

    security_util_8_to_6_encode(
        (const uint8_t *) plug_mcode, out->packed_payload);
    memset(out->zeros, 0, sizeof(out->zeros));

    security_rp2_create_signature(
        plug_id_reversed, sign_key_tmp, out->signature);
    out->crc = crc8((uint8_t *) out, sizeof(*out) - 1, 0);
}
