#ifndef BT_API_IO_IIDX_H
#define BT_API_IO_IIDX_H

/* IO emulation provider for beatmania IIDX. */

#include <stdbool.h>
#include <stdint.h>

/* Bit mapping for the "pad" word */

typedef enum bt_io_iidx_sys_bit {
    BT_IO_IIDX_SYS_TEST = 0x00,
    BT_IO_IIDX_SYS_SERVICE = 0x01,
    BT_IO_IIDX_SYS_COIN = 0x02
} bt_io_iidx_sys_bit_t;

typedef enum bt_io_iidx_panel_bit {
    BT_IO_IIDX_PANEL_P1_START = 0x00,
    BT_IO_IIDX_PANEL_P2_START = 0x01,
    BT_IO_IIDX_PANEL_VEFX = 0x02,
    BT_IO_IIDX_PANEL_EFFECT = 0x03
} bt_io_iidx_panel_bit_t;

typedef enum bt_io_iidx_key_bit {
    BT_IO_IIDX_KEY_P1_1 = 0x00,
    BT_IO_IIDX_KEY_P1_2 = 0x01,
    BT_IO_IIDX_KEY_P1_3 = 0x02,
    BT_IO_IIDX_KEY_P1_4 = 0x03,
    BT_IO_IIDX_KEY_P1_5 = 0x04,
    BT_IO_IIDX_KEY_P1_6 = 0x05,
    BT_IO_IIDX_KEY_P1_7 = 0x06,

    BT_IO_IIDX_KEY_P2_1 = 0x07,
    BT_IO_IIDX_KEY_P2_2 = 0x08,
    BT_IO_IIDX_KEY_P2_3 = 0x09,
    BT_IO_IIDX_KEY_P2_4 = 0x0A,
    BT_IO_IIDX_KEY_P2_5 = 0x0B,
    BT_IO_IIDX_KEY_P2_6 = 0x0C,
    BT_IO_IIDX_KEY_P2_7 = 0x0D
} bt_io_iidx_key_bit_t;

/* Bit mapping for the P1 and P2 deck lights */

typedef enum bt_io_iidx_deck_light {
    BT_IO_IIDX_DECK_LIGHT_P1_1 = 0,
    BT_IO_IIDX_DECK_LIGHT_P1_2 = 1,
    BT_IO_IIDX_DECK_LIGHT_P1_3 = 2,
    BT_IO_IIDX_DECK_LIGHT_P1_4 = 3,
    BT_IO_IIDX_DECK_LIGHT_P1_5 = 4,
    BT_IO_IIDX_DECK_LIGHT_P1_6 = 5,
    BT_IO_IIDX_DECK_LIGHT_P1_7 = 6,

    BT_IO_IIDX_DECK_LIGHT_P2_1 = 8,
    BT_IO_IIDX_DECK_LIGHT_P2_2 = 9,
    BT_IO_IIDX_DECK_LIGHT_P2_3 = 10,
    BT_IO_IIDX_DECK_LIGHT_P2_4 = 11,
    BT_IO_IIDX_DECK_LIGHT_P2_5 = 12,
    BT_IO_IIDX_DECK_LIGHT_P2_6 = 13,
    BT_IO_IIDX_DECK_LIGHT_P2_7 = 14,
} bt_io_iidx_deck_light_t;

/* Bit mapping for the front panel lights */

typedef enum bt_io_iidx_panel_light {
    BT_IO_IIDX_PANEL_LIGHT_P1_START = 0,
    BT_IO_IIDX_PANEL_LIGHT_P2_START = 1,
    BT_IO_IIDX_PANEL_LIGHT_VEFX = 2,
    BT_IO_IIDX_PANEL_LIGHT_EFFECT = 3,
} bt_io_iidx_panel_light_t;

/* Bit mapping for the top lamps from left to right when facing cabinet screen
 */

typedef enum bt_io_iidx_top_lamp {
    BT_IO_IIDX_TOP_LAMP_LEFT_BLUE = 0,
    BT_IO_IIDX_TOP_LAMP_LEFT_GREEN = 1,
    BT_IO_IIDX_TOP_LAMP_LEFT_YELLOW = 2,
    BT_IO_IIDX_TOP_LAMP_LEFT_RED = 3,
    BT_IO_IIDX_TOP_LAMP_RIGHT_BLUE = 4,
    BT_IO_IIDX_TOP_LAMP_RIGHT_GREEN = 5,
    BT_IO_IIDX_TOP_LAMP_RIGHT_YELLOW = 6,
    BT_IO_IIDX_TOP_LAMP_RIGHT_RED = 7,
} bt_io_iidx_top_lamp_t;

typedef bool (*bt_io_iidx_init_t)();
typedef void (*bt_io_iidx_fini_t)();
typedef void (*bt_io_iidx_ep1_deck_lights_set_t)(uint16_t deck_lights);
typedef void (*bt_io_iidx_ep1_panel_lights_set_t)(uint8_t panel_lights);
typedef void (*bt_io_iidx_ep1_top_lamps_set_t)(uint8_t top_lamps);
typedef void (*bt_io_iidx_ep1_top_neons_set_t)(bool top_neons);
typedef bool (*bt_io_iidx_ep1_send_t)();
typedef bool (*bt_io_iidx_ep2_recv_t)();
typedef uint8_t (*bt_io_iidx_ep2_turntable_get_t)(uint8_t player_no);
typedef uint8_t (*bt_io_iidx_ep2_slider_get_t)(uint8_t slider_no);
typedef uint8_t (*bt_io_iidx_ep2_sys_get_t)();
typedef uint8_t (*bt_io_iidx_ep2_panel_get_t)();
typedef uint16_t (*bt_io_iidx_ep2_keys_get_t)();
typedef bool (*bt_io_iidx_ep3_16seg_send_t)(const char *text);

typedef struct bt_io_iidx_api {
    uint16_t version;

    struct {
        // Required to be implemented
        bt_io_iidx_init_t init;
        bt_io_iidx_fini_t fini;
        bt_io_iidx_ep1_deck_lights_set_t ep1_deck_lights_set;
        bt_io_iidx_ep1_panel_lights_set_t ep1_panel_lights_set;
        bt_io_iidx_ep1_top_lamps_set_t ep1_top_lamps_set;
        bt_io_iidx_ep1_top_neons_set_t ep1_top_neons_set;
        bt_io_iidx_ep1_send_t ep1_send;
        bt_io_iidx_ep2_recv_t ep2_recv;
        bt_io_iidx_ep2_turntable_get_t ep2_turntable_get;
        bt_io_iidx_ep2_slider_get_t ep2_slider_get;
        bt_io_iidx_ep2_sys_get_t ep2_sys_get;
        bt_io_iidx_ep2_panel_get_t ep2_panel_get;
        bt_io_iidx_ep2_keys_get_t ep2_keys_get;
        bt_io_iidx_ep3_16seg_send_t ep3_16seg_send;
    } v1;
} bt_io_iidx_api_t;

#endif
