#include "cmd.h"

uint8_t extio_cmd_checksum(const struct extio_cmd_write *write)
{
    const uint8_t *data;
    uint32_t checksum;

    data = (const uint8_t *) write;

    checksum = 0;
    checksum += data[0];
    checksum += data[1];
    checksum += data[2];

    return (uint8_t) (checksum & 0x7F);
}