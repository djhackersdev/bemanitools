#ifndef BT_API_IO_JB_H
#define BT_API_IO_JB_H

/* IO emulation provider for jubeat. */

#include <stdbool.h>
#include <stdint.h>

/* input bit mappings. Panels on the controller are
    panel 1 top left corner down to panel 16 bottom right corner */
typedef enum bt_io_jb_panel_bit {
    BT_IO_JB_PANEL_01 = 0x00,
    BT_IO_JB_PANEL_02 = 0x01,
    BT_IO_JB_PANEL_03 = 0x02,
    BT_IO_JB_PANEL_04 = 0x03,
    BT_IO_JB_PANEL_05 = 0x04,
    BT_IO_JB_PANEL_06 = 0x05,
    BT_IO_JB_PANEL_07 = 0x06,
    BT_IO_JB_PANEL_08 = 0x07,
    BT_IO_JB_PANEL_09 = 0x08,
    BT_IO_JB_PANEL_10 = 0x09,
    BT_IO_JB_PANEL_11 = 0x0A,
    BT_IO_JB_PANEL_12 = 0x0B,
    BT_IO_JB_PANEL_13 = 0x0C,
    BT_IO_JB_PANEL_14 = 0x0D,
    BT_IO_JB_PANEL_15 = 0x0E,
    BT_IO_JB_PANEL_16 = 0x0F,
} bt_io_jb_panel_bit_t;

/* input "single button mode" mappings. Allows you to check each corner of each
    button to determine any flaky inputs
*/
typedef enum bt_io_jb_panel_mode {
    BT_IO_JB_PANEL_MODE_ALL = 0, // any of the four corners will trigger a panel
    BT_IO_JB_PANEL_MODE_TOP_LEFT = 1,
    BT_IO_JB_PANEL_MODE_TOP_RIGHT = 2,
    BT_IO_JB_PANEL_MODE_BOTTOM_RIGHT = 3,
    BT_IO_JB_PANEL_MODE_BOTTOM_LEFT = 4,
} bt_io_jb_panel_mode_t;

/* Bit mappings for "system" inputs */
typedef enum bt_io_jb_sys_bit {
    BT_IO_JB_SYS_TEST = 0x00,
    BT_IO_JB_SYS_SERVICE = 0x01,
    BT_IO_JB_SYS_COIN = 0x02,
} bt_io_jb_sys_bit_t;

/* RGB led units to address */
typedef enum bt_io_jb_rgb_led {
    BT_IO_JB_RGB_LED_FRONT = 0,
    BT_IO_JB_RGB_LED_TOP = 1,
    BT_IO_JB_RGB_LED_LEFT = 2,
    BT_IO_JB_RGB_LED_RIGHT = 3,
    BT_IO_JB_RGB_LED_TITLE = 4,
    BT_IO_JB_RGB_LED_WOOFER = 5
} bt_io_jb_rgb_led_t;

typedef bool (*bt_io_jb_init_t)();
typedef void (*bt_io_jb_fini_t)();
typedef bool (*bt_io_jb_inputs_read_t)();
typedef uint8_t (*bt_io_jb_sys_inputs_get_t)();
typedef uint16_t (*bt_io_jb_panel_inputs_get_t)();
typedef void (*bt_io_jb_rgb_led_set_t)(bt_io_jb_rgb_led_t unit, uint8_t r, uint8_t g, uint8_t b);
typedef bool (*bt_io_jb_lights_write_t)();
typedef bool (*bt_io_jb_panel_mode_set_t)(bt_io_jb_panel_mode_t mode);
typedef bool (*bt_io_jb_coin_blocker_set_t)(bool blocked);

typedef struct bt_io_jb_api {
    uint16_t version;

    struct {
        // Required to be implemented
        bt_io_jb_init_t init;
        bt_io_jb_fini_t fini;
        bt_io_jb_inputs_read_t inputs_read;
        bt_io_jb_sys_inputs_get_t sys_inputs_get;
        bt_io_jb_panel_inputs_get_t panel_inputs_get;
        bt_io_jb_rgb_led_set_t rgb_led_set;
        bt_io_jb_lights_write_t lights_write;
        bt_io_jb_panel_mode_set_t panel_mode_set;
        bt_io_jb_coin_blocker_set_t coin_blocker_set;
    } v1;
} bt_io_jb_api_t;

#endif
