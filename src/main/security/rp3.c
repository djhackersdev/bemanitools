#include <string.h>

#include "iface-core/log.h"

#include "security/rp-util.h"
#include "security/rp2.h"
#include "security/rp3.h"
#include "security/util.h"

#include "util/crc.h"

void security_rp3_generate_signed_eeprom_data(
    enum security_rp_util_rp_type type,
    const struct security_rp_sign_key *sign_key,
    const struct security_mcode *plug_mcode,
    const struct security_id *plug_id,
    struct security_rp3_eeprom *out)
{
    uint8_t sign_key_tmp[8];
    uint8_t plug_id_reversed[8];
    const uint8_t *plug_mcode_raw = (const uint8_t *) plug_mcode;

    log_assert(sign_key);
    log_assert(plug_mcode);
    log_assert(plug_id);
    log_assert(out);

    memcpy(sign_key_tmp, sign_key, sizeof(sign_key_tmp));
    if (type == SECURITY_RP_UTIL_RP_TYPE_BLACK) {
        for (int i = 0; i < sizeof(sign_key_tmp); i++) {
            sign_key_tmp[i] ^= plug_mcode_raw[i];
        }
    }

    for (int i = 0; i < sizeof(plug_id_reversed); i++) {
        plug_id_reversed[i] = plug_id->id[7 - i];
    }

    security_rp2_generate_signed_eeprom_data_core(
        sign_key_tmp,
        plug_mcode_raw,
        plug_id_reversed,
        out->signature,
        out->packed_payload);

    memset(out->zeros, 0, sizeof(out->zeros));
    out->crc = crc8((uint8_t *) out, sizeof(*out) - 1, 0);
}
