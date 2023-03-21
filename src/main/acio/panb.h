#ifndef AC_IO_PANB_H
#define AC_IO_PANB_H

#include <stdint.h>

#define AC_IO_CMD_PANB_POLL_REPLY 0x0110
#define AC_IO_CMD_PANB_SEND_LAMP 0x0111
#define AC_IO_CMD_PANB_START_AUTO_INPUT 0x0115

#define AC_IO_PANB_NUM_NODES 4
#define AC_IO_PANB_MAX_KEYS (7 * AC_IO_PANB_NUM_NODES)
#define AC_IO_PANB_MAX_KEYPAIRS (AC_IO_PANB_MAX_KEYS / 2)

struct ac_io_panb_keypair {
    uint8_t key2 : 4;
    uint8_t key1 : 4;
};

#pragma pack(push, 1)
struct ac_io_panb_poll_in {
    /* last received command sequence number (start_auto_input / send_lamp) */
    uint8_t sub_seq1;
    /* auto-increment sequence number for autopoll reports */
    uint8_t sub_seq2;
    struct ac_io_panb_keypair keypair[AC_IO_PANB_MAX_KEYPAIRS];
};
#pragma pack(pop)

struct ac_io_panb_color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

#pragma pack(push, 1)
struct ac_io_panb_poll_out {
    struct ac_io_panb_color key[AC_IO_PANB_MAX_KEYS];
};
#pragma pack(pop)

#endif
