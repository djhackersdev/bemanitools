#ifndef GENINPUT_HID_META_OUT_H
#define GENINPUT_HID_META_OUT_H

#include <hidsdi.h>
#include <windows.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "geninput/hid-report-out.h"
#include "geninput/hid.h"

struct hid_out_light {
    struct hid_report_out *report;
    uint16_t collection_id;
    uint16_t size;
};

struct hid_meta_out {
    PHIDP_PREPARSED_DATA ppd;
    HIDP_CAPS caps_tlc;
    HIDP_BUTTON_CAPS *caps_btn;
    HIDP_VALUE_CAPS *caps_val;

    struct hid_light *lights;
    size_t nlights;
    size_t nbuttons;

    struct hid_out_light *priv_lights;
    struct hid_report_out *reports;
    uint8_t nreports;
    uint8_t next_report;
};

bool hid_meta_out_init(
    struct hid_meta_out *meta, PHIDP_PREPARSED_DATA ppd, HANDLE fd);
void hid_meta_out_get_next_report(struct hid_meta_out *meta, uint8_t *bytes);
size_t hid_meta_out_get_buffer_size(const struct hid_meta_out *meta);
const struct hid_light *
hid_meta_out_get_lights(const struct hid_meta_out *meta);
size_t hid_meta_out_get_nlights(const struct hid_meta_out *meta);
bool hid_meta_out_set_light(
    struct hid_meta_out *meta, size_t light_no, uint32_t intensity);
void hid_meta_out_fini(struct hid_meta_out *meta);

#endif
