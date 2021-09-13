#ifndef AC_IO_MDXF_H
#define AC_IO_MDXF_H

#include <stdint.h>

#define AC_IO_CMD_MDXF_POLL 0x0110
#define AC_IO_CMD_MDXF_AUTO_GET_START 0x0116
#define AC_IO_CMD_MDXF_LIGHT 0x0113

#pragma pack(push, 1)

union mdxf_panel_sensors_individual {
    struct {
        uint8_t right : 1;
        uint8_t left : 1;
        uint8_t down : 1;
        uint8_t up : 1;
    };
    uint8_t raw;
};

struct mdxf_panel_sensors {
    uint8_t down : 4;
    uint8_t up : 4;
    uint8_t right : 4;
    uint8_t left : 4;
};

struct ac_io_mdxf_poll_in {
    struct mdxf_panel_sensors panel;
    uint8_t unk;
};

struct mdxf_lights {
    uint8_t up_right;
    uint8_t down_left;
    uint8_t up_left;
    uint8_t down_right;
    uint8_t unk;
};

struct ac_io_mdxf_light_out {
    struct mdxf_lights side[2];
};

#pragma pack(pop)

#endif
