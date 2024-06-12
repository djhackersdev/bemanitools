#ifndef EAMIO_CONFIG_H
#define EAMIO_CONFIG_H

#include "api/io/eam.h"

struct bt_io_eam_config_api {
    void (*config_save)(void);
    bool (*get_autogen)(void);
    void (*set_autogen)(bool autogen);
    bool (*get_alt_10k)(void);
    void (*set_alt_10k)(bool alt_10k);
    struct hid_stub *(*get_keypad_device)(uint8_t unit_no);
    void (*set_keypad_device)(uint8_t unit_no, struct hid_stub *hid);
    const char *(*get_card_path)(uint8_t unit_no);
    void (*set_card_path)(uint8_t unit_no, const char *path);
};

#endif
