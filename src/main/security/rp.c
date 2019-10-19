#include "security/rp.h"

#include "security/rp-blowfish.h"
#include "security/rp-enc-table.h"
#include "security/util.h"

#include "util/crypto.h"
#include "util/log.h"

static uint32_t security_rp_get_len_mcode(const struct security_mcode *mcode)
{
    uint32_t len;

    len = 0;

    while (len < sizeof(struct security_mcode) &&
           ((const char *) mcode)[len] != ' ') {
        len++;
    }

    return len;
}

void security_rp_generate_signed_eeprom_data(
    const struct security_mcode *boot_version,
    const uint32_t *boot_seeds,
    const struct security_mcode *plug_mcode,
    const struct security_id *plug_id,
    struct security_rp_eeprom *out)
{
    uint8_t encryption_key[sizeof(security_rp_enc_table_key_base)];
    uint32_t boot_version_len;
    uint32_t idx;
    uint32_t seed;
    uint8_t *enc_key_section1;
    uint8_t *enc_key_section2;
    uint8_t data[32];
    struct blowfish ctx;

    log_assert(boot_version);
    log_assert(boot_seeds);
    log_assert(plug_mcode);
    log_assert(plug_id);
    log_assert(out);

    log_assert(boot_seeds[0] <= 16);
    log_assert(boot_seeds[1] <= 16);
    log_assert(boot_seeds[2] >= boot_seeds[1]);

    memcpy(
        encryption_key,
        security_rp_enc_table_key_base,
        sizeof(security_rp_enc_table_key_base));

    boot_version_len = security_rp_get_len_mcode(boot_version);

    idx = 0;

    for (uint32_t i = 0; i < sizeof(encryption_key); i++) {
        encryption_key[i] ^= ((const char *) boot_version)[idx];
        idx = (idx + 1) % boot_version_len;
    }

    seed = 16 * (boot_seeds[2] + 16 * (boot_seeds[1] + 16 * boot_seeds[0]));
    enc_key_section1 = (uint8_t *) encryption_key;
    enc_key_section2 = (uint8_t *) (((uint8_t *) encryption_key) + 14);

    memset(data, 0, sizeof(data));

    data[0] = enc_key_section2[seed];
    data[1] = enc_key_section2[seed + 1];
    data[2] = plug_id->id[7];
    data[3] = plug_id->id[6];
    data[4] = plug_id->id[5];
    data[5] = plug_id->id[4];
    data[6] = plug_id->id[3];
    data[7] = plug_id->id[2];

    security_rp_blowfish_init(&ctx, enc_key_section1, 14, seed);
    security_rp_blowfish_enc(&ctx, data, &data[16], 8);

    for (uint8_t i = 0; i < sizeof(out->signature); i++) {
        out->signature[i] = data[i + 16] ^ data[i + 22];
    }

    security_util_8_to_6_encode_reverse(
        (const uint8_t *) plug_mcode, out->packed_payload);
}