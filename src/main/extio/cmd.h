#ifndef EXTIO_CMD_H
#define EXTIO_CMD_H

#include <stdint.h>

#define EXTIO_PAD_LIGHT_MAX_PLAYERS 2

enum extio_status {
    EXTIO_STATUS_OK = 0x11,
};

#pragma pack(push, 1)

struct extio_cmd_pad_light {
    uint8_t unknown : 3;
    uint8_t right : 1;
    uint8_t left : 1;
    uint8_t down : 1;
    uint8_t up : 1;
    uint8_t unknown_80 : 1;
};

struct extio_cmd_write {
    struct extio_cmd_pad_light pad_lights[EXTIO_PAD_LIGHT_MAX_PLAYERS];

    uint8_t unknown3 : 3;
    uint8_t sensor_read_mode : 3;
    uint8_t neons : 1;
    uint8_t unknown4 : 1;

    uint8_t checksum;
};

struct extio_cmd_read {
    uint8_t status;
};

_Static_assert(
    sizeof(struct extio_cmd_write) == 4,
    "struct extio_cmd_write is the wrong size");
_Static_assert(
    sizeof(struct extio_cmd_read) == 1,
    "struct extio_cmd_read is the wrong size");

#pragma pack(pop)

uint8_t extio_cmd_checksum(const struct extio_cmd_write *write);

#endif