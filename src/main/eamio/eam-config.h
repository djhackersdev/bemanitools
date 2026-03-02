#ifndef EAMIO_CONFIG_H
#define EAMIO_CONFIG_H

#include "bemanitools/eamio.h"

struct eam_io_config_api {
    void (*config_save)(void);
    bool (*get_autogen)(void);
    void (*set_autogen)(bool autogen);
    bool (*get_alt_10k)(uint8_t unit_no);
    void (*set_alt_10k)(uint8_t unit_no, bool alt_10k);
    struct hid_stub *(*get_keypad_device)(uint8_t unit_no);
    void (*set_keypad_device)(uint8_t unit_no, struct hid_stub *hid);
    const char *(*get_card_path)(uint8_t unit_no);
    void (*set_card_path)(uint8_t unit_no, const char *path);
    bool (*get_scard)(void);
    void (*set_scard)(bool scard);
    bool (*get_mux_keypad)(void);
    void (*set_mux_keypad)(bool mux_keypad);
    bool (*get_mux_scard)(void);
    void (*set_mux_scard)(bool mux_scard);
    
    bool (*do_keypads_conflict)(void);
    LPSTR (*get_scard_reader_name)(uint8_t unit_no);
};

#endif
