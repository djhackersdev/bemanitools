#ifndef BT_API_IO_DDR_H
#define BT_API_IO_DDR_H

#include <stdbool.h>
#include <stdint.h>

typedef enum bt_io_ddr_pad_bit {
    BT_IO_DDR_TEST = 0x04,
    BT_IO_DDR_COIN = 0x05,
    BT_IO_DDR_SERVICE = 0x06,

    BT_IO_DDR_P2_START = 0x08,
    BT_IO_DDR_P2_UP = 0x09,
    BT_IO_DDR_P2_DOWN = 0x0A,
    BT_IO_DDR_P2_LEFT = 0x0B,
    BT_IO_DDR_P2_RIGHT = 0x0C,
    BT_IO_DDR_P2_MENU_LEFT = 0x0E,
    BT_IO_DDR_P2_MENU_RIGHT = 0x0F,
    BT_IO_DDR_P2_MENU_UP = 0x02,
    BT_IO_DDR_P2_MENU_DOWN = 0x03,

    BT_IO_DDR_P1_START = 0x10,
    BT_IO_DDR_P1_UP = 0x11,
    BT_IO_DDR_P1_DOWN = 0x12,
    BT_IO_DDR_P1_LEFT = 0x13,
    BT_IO_DDR_P1_RIGHT = 0x14,
    BT_IO_DDR_P1_MENU_LEFT = 0x16,
    BT_IO_DDR_P1_MENU_RIGHT = 0x17,
    BT_IO_DDR_P1_MENU_UP = 0x00,
    BT_IO_DDR_P1_MENU_DOWN = 0x01,
} bt_io_ddr_pad_bit_t;

// see the functions below for more information

typedef enum bt_io_ddr_p3io_light_bit {
    BT_IO_DDR_P3IO_LIGHT_P1_MENU = 0x00,
    BT_IO_DDR_P3IO_LIGHT_P2_MENU = 0x01,
    BT_IO_DDR_P3IO_LIGHT_P2_LOWER_LAMP = 0x04,
    BT_IO_DDR_P3IO_LIGHT_P2_UPPER_LAMP = 0x05,
    BT_IO_DDR_P3IO_LIGHT_P1_LOWER_LAMP = 0x06,
    BT_IO_DDR_P3IO_LIGHT_P1_UPPER_LAMP = 0x07,
} bt_io_ddr_p3io_light_bit_t;

typedef enum bt_io_ddr_hdxs_light_bit {
    BT_IO_DDR_HDXS_LIGHT_HD_P1_START = 0x08,
    BT_IO_DDR_HDXS_LIGHT_HD_P1_UP_DOWN = 0x09,
    BT_IO_DDR_HDXS_LIGHT_HD_P1_LEFT_RIGHT = 0x0A,
    BT_IO_DDR_HDXS_LIGHT_HD_P2_START = 0x0B,
    BT_IO_DDR_HDXS_LIGHT_HD_P2_UP_DOWN = 0x0C,
    BT_IO_DDR_HDXS_LIGHT_HD_P2_LEFT_RIGHT = 0x0D,
} bt_io_ddr_hdxs_light_bit_t;

// the indexing starts from 0x20 if you're looking in geninput
typedef enum bt_io_ddr_hdxs_rgb_light_idx {
    BT_IO_DDR_HDXS_LIGHT_HD_P1_SPEAKER_F_R = 0x00,
    BT_IO_DDR_HDXS_LIGHT_HD_P1_SPEAKER_F_G = 0x01,
    BT_IO_DDR_HDXS_LIGHT_HD_P1_SPEAKER_F_B = 0x02,
    BT_IO_DDR_HDXS_LIGHT_HD_P2_SPEAKER_F_R = 0x03,
    BT_IO_DDR_HDXS_LIGHT_HD_P2_SPEAKER_F_G = 0x04,
    BT_IO_DDR_HDXS_LIGHT_HD_P2_SPEAKER_F_B = 0x05,
    BT_IO_DDR_HDXS_LIGHT_HD_P1_SPEAKER_W_R = 0x06,
    BT_IO_DDR_HDXS_LIGHT_HD_P1_SPEAKER_W_G = 0x07,
    BT_IO_DDR_HDXS_LIGHT_HD_P1_SPEAKER_W_B = 0x08,
    BT_IO_DDR_HDXS_LIGHT_HD_P2_SPEAKER_W_R = 0x09,
    BT_IO_DDR_HDXS_LIGHT_HD_P2_SPEAKER_W_G = 0x0A,
    BT_IO_DDR_HDXS_LIGHT_HD_P2_SPEAKER_W_B = 0x0B,
} bt_io_ddr_hdxs_rgb_light_idx_t;

typedef enum bt_io_ddr_extio_light_bit {
    BT_IO_DDR_EXTIO_LIGHT_NEONS = 0x0E,

    BT_IO_DDR_EXTIO_LIGHT_P2_RIGHT = 0x13,
    BT_IO_DDR_EXTIO_LIGHT_P2_LEFT = 0x14,
    BT_IO_DDR_EXTIO_LIGHT_P2_DOWN = 0x15,
    BT_IO_DDR_EXTIO_LIGHT_P2_UP = 0x16,

    BT_IO_DDR_EXTIO_LIGHT_P1_RIGHT = 0x1B,
    BT_IO_DDR_EXTIO_LIGHT_P1_LEFT = 0x1C,
    BT_IO_DDR_EXTIO_LIGHT_P1_DOWN = 0x1D,
    BT_IO_DDR_EXTIO_LIGHT_P1_UP = 0x1E
} bt_io_ddr_extio_light_bit_t;

typedef bool (*bt_io_ddr_init_t)();
typedef void (*bt_io_ddr_fini_t)();
typedef uint32_t (*bt_io_ddr_pad_read_t)();
typedef void (*bt_io_ddr_extio_lights_set_t)(uint32_t extio_lights);
typedef void (*bt_io_ddr_p3io_lights_set_t)(uint32_t p3io_lights);
typedef void (*bt_io_ddr_hdxs_lights_panel_set_t)(uint32_t hdxs_lights);
typedef void (*bt_io_ddr_hdxs_lights_rgb_set_t)(uint8_t idx, uint8_t r, uint8_t g, uint8_t b);

typedef struct bt_io_ddr_api {
    uint16_t version;

    struct {
        // Required to be implemented
        bt_io_ddr_init_t init;
        bt_io_ddr_fini_t fini;
        bt_io_ddr_pad_read_t pad_read;
        bt_io_ddr_extio_lights_set_t extio_lights_set;
        bt_io_ddr_p3io_lights_set_t p3io_lights_set;
        bt_io_ddr_hdxs_lights_panel_set_t hdxs_lights_panel_set;
        bt_io_ddr_hdxs_lights_rgb_set_t hdxs_lights_rgb_set;
    } v1;
} bt_io_ddr_api_t;

#endif
