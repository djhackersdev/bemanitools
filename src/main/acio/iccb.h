#ifndef AC_IO_ICCB_H
#define AC_IO_ICCB_H

#include <stdint.h>

enum ac_io_iccb_cmd {
    /* found on jubeat prop, sent after acio init req, maybe fw update? */
    AC_IO_ICCB_CMD_UNK_0100         = 0x0100,
    /* found on jubeat prop, sent right after queue loop start */
    AC_IO_ICCB_CMD_UNK_0116         = 0x0116,
    /* found on jubeat prop, sent after 0100 req */
    AC_IO_ICCB_CMD_UNK_0120         = 0x0120,
    AC_IO_ICCB_CMD_QUEUE_LOOP_START = 0x0130,
    AC_IO_ICCB_CMD_POLL             = 0x0134,
    AC_IO_ICCB_CMD_UNK_135          = 0x0135,
    AC_IO_ICCB_CMD_SLEEP            = 0x013A,
    AC_IO_ICCB_CMD_READ_CARD        = 0x0161
};

enum ac_io_iccb_sensor_state {
    AC_IO_ICCB_SENSOR_CARD = 0x02,
    AC_IO_ICCB_SENSOR_NO_CARD = 0x04
};

enum ac_io_iccb_card_type {
    AC_IO_ICCB_CARD_TYPE_ISO15696 = 0x0,
    AC_IO_ICCB_CARD_TYPE_FELICA = 0x1,
};

#pragma pack(push, 1)

struct ac_io_iccb_misc {
    uint8_t unknown;
    uint8_t subcmd;
};

struct ac_io_iccb_state {
    uint8_t sensor_state;
    uint8_t card_type;
    uint8_t uid[8];
    uint8_t unk2;
    uint8_t unk3;
    uint8_t unk4[4];
};

#pragma pack(pop)

#endif