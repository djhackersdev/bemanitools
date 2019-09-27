#ifndef GENINPUT_HID_REPORT_IN_H
#define GENINPUT_HID_REPORT_IN_H

#include <windows.h>
#include <hidpi.h>
#include <hidsdi.h>

#include <stdint.h>

struct hid_report_in {
    uint8_t id;
    uint32_t nbuttons;
    uint32_t *buttons;
    uint8_t *bytes;
    uint32_t nbytes;
};

void hid_report_in_init(struct hid_report_in *r, uint8_t report_id,
        uint32_t nbuttons, void *bytes, size_t nbytes,
        PHIDP_PREPARSED_DATA ppd);
uint8_t hid_report_in_get_id(struct hid_report_in *r);
bool hid_report_in_exchange(struct hid_report_in *r, uint8_t **bytes,
        uint32_t nbytes);
bool hid_report_in_get_bit(struct hid_report_in *r, PHIDP_PREPARSED_DATA ppd,
        uint16_t collection_id, uint32_t usage, int32_t *value);
bool hid_report_in_get_value(struct hid_report_in *r, PHIDP_PREPARSED_DATA ppd,
        uint16_t collection_id, uint32_t usage, int32_t *value);
void hid_report_in_fini(struct hid_report_in *r);

#endif
