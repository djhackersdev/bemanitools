#include <string.h>

#include "iface-core/log.h"

#include "security/id.h"

#include "util/hex.h"
#include "util/mem.h"

const struct security_id security_id_default = {
    .header = SECURITY_ID_HEADER,
    .id = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    .checksum = 0x6F,
};

static uint8_t
security_id_checksum_calc(uint8_t initseed, const uint8_t *inbuf, size_t length)
{
    const uint8_t *v3; // edi@1
    int v4; // ebp@2
    uint8_t result; // al@2
    unsigned int v6; // esi@3
    signed int v7; // ebx@3
    int v8; // edx@4
    uint8_t v9; // al@4
    int v10; // edx@6
    uint8_t v11; // al@6
    int v12; // edx@8
    uint8_t v13; // al@8
    int v14; // edx@10

    v3 = inbuf;

    if (length <= 0) {
        result = initseed;
    } else {
        v4 = length;
        result = initseed;

        do {
            v6 = *v3;
            v7 = 0;

            do {
                v8 = (result ^ (uint8_t) (v6 >> v7)) & 1;
                v9 = result >> 1;

                if (v8) {
                    v9 ^= 0x8Cu;
                }

                v10 = (v9 ^ (uint8_t) (v6 >> (v7 + 1))) & 1;
                v11 = v9 >> 1;

                if (v10) {
                    v11 ^= 0x8Cu;
                }

                v12 = (v11 ^ (uint8_t) (v6 >> (v7 + 2))) & 1;
                v13 = v11 >> 1;

                if (v12) {
                    v13 ^= 0x8Cu;
                }

                v14 = (v13 ^ (uint8_t) (v6 >> (v7 + 3))) & 1;
                result = v13 >> 1;

                if (v14) {
                    result ^= 0x8Cu;
                }

                v7 += 4;
            } while (v7 < 8);

            v3 = (v3 + 1);
            --v4;
        } while (v4);
    }

    return result;
}

static uint8_t security_id_checksum_buffer(const uint8_t *inbuf)
{
    uint8_t bufcheck[7];

    bufcheck[0] = inbuf[1];
    bufcheck[1] = inbuf[7];
    bufcheck[2] = inbuf[6];
    bufcheck[3] = inbuf[5];
    bufcheck[4] = inbuf[4];
    bufcheck[5] = inbuf[3];
    bufcheck[6] = inbuf[2];

    return security_id_checksum_calc(0, bufcheck, sizeof(bufcheck));
}

bool security_id_parse(const char *str, struct security_id *id)
{
    log_assert(str);
    log_assert(id);

    return hex_decode(id, sizeof(struct security_id), str, strlen(str));
}

char *security_id_to_str(const struct security_id *id, bool id_only)
{
    char *str;
    size_t len;

    log_assert(id);

    if (id_only) {
        len = sizeof(id->id) * 2 + 1;
        str = xmalloc(len);
        hex_encode_uc(id->id, sizeof(id->id), str, len);
    } else {
        len = sizeof(struct security_id) * 2 + 1;
        str = xmalloc(len);
        hex_encode_uc(id, sizeof(*id), str, len);
    }

    str[len - 1] = '\0';

    return str;
}

void security_id_prepare(struct security_id *id)
{
    log_assert(id);

    id->header = SECURITY_ID_HEADER;
    id->checksum = security_id_checksum_buffer(id->id);
}

bool security_id_verify(const struct security_id *id)
{
    log_assert(id);

    if (id->header != SECURITY_ID_HEADER) {
        log_warning(
            "PCBID header needs to be %02X but was %02X",
            SECURITY_ID_HEADER,
            id->header);
        return false;
    }

    uint8_t check = id->checksum;
    uint8_t need = security_id_checksum_buffer(id->id);

    if (check != need) {
        log_warning("PCBID checksum should be %02X got %02X", need, check);
        return false;
    }

    return true;
}
