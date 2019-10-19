#ifndef AC_IO_ICCA_H
#define AC_IO_ICCA_H

#include <stdint.h>

enum ac_io_icca_cmd {
    /* Yet unknown command encountered first on jubeat (1) */
    AC_IO_ICCA_CMD_UNKN_0120 = 0x0120,
    AC_IO_ICCA_CMD_QUEUE_LOOP_START = 0x0130,
    AC_IO_ICCA_CMD_ENGAGE = 0x0131,
    AC_IO_ICCA_CMD_POLL = 0x0134,
    AC_IO_ICCA_CMD_SET_SLOT_STATE = 0x0135,
    AC_IO_ICCA_CMD_BEGIN_KEYPAD = 0x013A,
    AC_IO_ICCA_CMD_POLL_FELICA = 0x0161,
};

enum ac_io_icca_slot_state {
    AC_IO_ICCA_SLOT_STATE_OPEN = 0x11,
    AC_IO_ICCA_SLOT_STATE_EJECT = 0x12,
    AC_IO_ICCA_SLOT_STATE_CLOSE = 0,
};

enum ac_io_icca_sensor_state {
    /* Card eject event fired once after slot state is set to eject the card */
    AC_IO_ICCA_SENSOR_STATE_CARD_EJECTED = 0x50,
    AC_IO_ICCA_SENSOR_MASK_FRONT_ON = (1 << 4),
    AC_IO_ICCA_SENSOR_MASK_BACK_ON = (1 << 5)
};

enum ac_io_icca_keypad_mask {
    AC_IO_ICCA_KEYPAD_MASK_EMPTY = (1 << 0),
    AC_IO_ICCA_KEYPAD_MASK_3 = (1 << 1),
    AC_IO_ICCA_KEYPAD_MASK_6 = (1 << 2),
    AC_IO_ICCA_KEYPAD_MASK_9 = (1 << 3),

    AC_IO_ICCA_KEYPAD_MASK_0 = (1 << 8),
    AC_IO_ICCA_KEYPAD_MASK_1 = (1 << 9),
    AC_IO_ICCA_KEYPAD_MASK_4 = (1 << 10),
    AC_IO_ICCA_KEYPAD_MASK_7 = (1 << 11),

    AC_IO_ICCA_KEYPAD_MASK_00 = (1 << 12),
    AC_IO_ICCA_KEYPAD_MASK_2 = (1 << 13),
    AC_IO_ICCA_KEYPAD_MASK_5 = (1 << 14),
    AC_IO_ICCA_KEYPAD_MASK_8 = (1 << 15),
};

#pragma pack(push, 1)

struct ac_io_icca_misc {
    uint8_t unknown;
    uint8_t subcmd;
};

struct ac_io_icca_state {
    /* Similar to the struct returned by libacio, but not quite the same */

    uint8_t status_code;
    uint8_t sensor_state;
    uint8_t uid[8];
    uint8_t card_type;
    uint8_t keypad_started;
    uint8_t key_events[2];
    uint16_t key_state;
};

#pragma pack(pop)

#endif
