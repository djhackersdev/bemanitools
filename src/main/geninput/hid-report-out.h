#ifndef GENINPUT_HID_REPORT_OUT_H
#define GENINPUT_HID_REPORT_OUT_H

#include <windows.h>
#include <hidsdi.h>

#include <stdbool.h>
#include <stdint.h>

struct hid_report_out {
    uint8_t id;
    uint8_t *bytes;
    size_t nbytes;
};

bool hid_report_out_init(struct hid_report_out *r, PHIDP_PREPARSED_DATA ppd,
        uint8_t id, size_t nbytes);
void hid_report_out_get_bytes(const struct hid_report_out *r, uint8_t *bytes);
bool hid_report_out_set_bit(struct hid_report_out *r,
        PHIDP_PREPARSED_DATA ppd, uint16_t collection_id, uint32_t usage,
        bool value);
bool hid_report_out_set_value(struct hid_report_out *r,
        PHIDP_PREPARSED_DATA ppd, uint16_t collection_id, uint32_t usage,
        uint32_t value);
void hid_report_out_fini(struct hid_report_out *r);

#endif
