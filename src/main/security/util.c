#include "security/util.h"

void security_util_8_to_6_encode(const uint8_t *in, uint8_t *out)
{
    uint8_t tmp[8];
    int i;

    for (i = 0; i < 8; i++) {
        tmp[i] = (in[i] - 0x20) & 0x3F;
    }

    out[0] = (tmp[0] >> 0) | (tmp[1] << 6);
    out[1] = (tmp[1] >> 2) | (tmp[2] << 4);
    out[2] = (tmp[2] >> 4) | (tmp[3] << 2);

    out[3] = (tmp[4] >> 0) | (tmp[5] << 6);
    out[4] = (tmp[5] >> 2) | (tmp[6] << 4);
    out[5] = (tmp[6] >> 4) | (tmp[7] << 2);
}

void security_util_6_to_8_decode(const uint8_t *in, uint8_t *out)
{
    int i;

    out[0] = ((in[0] >> 0) & 0x3F);
    out[1] = ((in[0] >> 6) & 0x03) | ((in[1] << 2) & 0x3C);
    out[2] = ((in[1] >> 4) & 0x0F) | ((in[2] << 4) & 0x30);
    out[3] = ((in[2] >> 2) & 0x3F);

    out[4] = ((in[3] >> 0) & 0x3F);
    out[5] = ((in[3] >> 6) & 0x03) | ((in[4] << 2) & 0x3C);
    out[6] = ((in[4] >> 4) & 0x0F) | ((in[5] << 4) & 0x30);
    out[7] = ((in[5] >> 2) & 0x3F);

    for (i = 0; i < 8; i++) {
        out[i] += 0x20;
    }
}

void security_util_8_to_6_encode_reverse(const uint8_t *in, uint8_t *out)
{
    out[0] = ((in[7] - 0x20) << 2) | (((in[6] - 0x20) >> 4) & 0x03);
    out[1] = ((in[6] - 0x20) << 4) | (((in[5] - 0x20) >> 2) & 0x0F);
    out[2] = ((in[5] - 0x20) << 6) | ((in[4] - 0x20) & 0x3F);
    out[3] = ((in[3] - 0x20) << 2) | (((in[2] - 0x20) >> 4) & 0x03);
    out[4] = ((in[2] - 0x20) << 4) | (((in[1] - 0x20) >> 2) & 0x0F);
    out[5] = ((in[1] - 0x20) << 6) | ((in[0] - 0x20) & 0x3F);
}

void security_util_6_to_8_decode_reverse(const uint8_t *in, uint8_t *out)
{
    out[0] = (in[5] & 0x3F) + 0x20;
    out[1] = ((in[5] >> 6) | (((in[4] & 0xF) << 2) + 0x20));
    out[2] = ((in[4] >> 4) | (((in[3] & 0x03) << 4) + 0x20));
    out[3] = (in[3] >> 2) + 0x20;
    out[4] = (in[2] & 0x3F) + 0x20;
    out[5] = (in[2] >> 6) | (((in[1] & 0x0F) << 2) + 0x20);
    out[6] = (in[1] >> 4) | (((in[0] & 0x03) << 4) + 0x20);
    out[7] = (in[0] >> 2) + 0x20;
}
