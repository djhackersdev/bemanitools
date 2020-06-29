#ifndef AC_IO_AC_IO_H
#define AC_IO_AC_IO_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "acio/hdxs.h"
#include "acio/icca.h"
#include "acio/kfca.h"

#define AC_IO_SOF 0xAA
#define AC_IO_ESCAPE 0xFF
#define AC_IO_RESPONSE_FLAG 0x80
#define AC_IO_BROADCAST 0x70

#define ac_io_u16(x) _byteswap_ushort(x)
#define ac_io_u32(x) _byteswap_ulong(x)

enum ac_io_cmd {
    AC_IO_CMD_ASSIGN_ADDRS = 0x0001,
    AC_IO_CMD_GET_VERSION = 0x0002,
    AC_IO_CMD_START_UP = 0x0003,
    AC_IO_CMD_KEEPALIVE = 0x0080,
    /* Yet unknown command encountered first on jubeat (1) */
    AC_IO_CMD_UNKN_00FF = 0x00FF,
    AC_IO_CMD_CLEAR = 0x0100,
};

enum ac_io_node_type {
    AC_IO_NODE_TYPE_H44B = 0x04010000,
    AC_IO_NODE_TYPE_ICCA = 0x03000000,
    /* same as ICCA */
    AC_IO_NODE_TYPE_ICCB = 0x03000000,
    AC_IO_NODE_TYPE_LED_STRIP = 0x04020000,
    AC_IO_NODE_TYPE_LED_SPIKE = 0x05010000,
    AC_IO_NODE_TYPE_KFCA = 0x09060000,
    AC_IO_NODE_TYPE_BI2A = 0x0d060000,
};

#pragma pack(push, 1)

struct ac_io_version {
    /* Names taken from some debug text in libacio.dll */
    uint32_t type;
    uint8_t flag;
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
    char product_code[4];
    char date[16];
    char time[16];
};

struct ac_io_message {
    uint8_t addr; /* High bit: clear = req, set = resp */

    union {
        struct {
            uint16_t code;
            uint8_t seq_no;
            uint8_t nbytes;

            union {
                uint8_t raw[0xFF];
                uint8_t count;
                uint8_t status;
                struct ac_io_version version;

                struct ac_io_icca_misc icca_misc;
                struct ac_io_icca_state icca_state;

                struct ac_io_kfca_poll_in kfca_poll_in;
                struct ac_io_kfca_poll_out kfca_poll_out;

                struct ac_io_hdxs_output hdxs_output;
            };
        } cmd;

        struct {
            uint8_t nbytes;
            uint8_t raw[0xFF]; /* 0xFFucked if I know */
        } bcast;
    };
};

#pragma pack(pop)

#endif
