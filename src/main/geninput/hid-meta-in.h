#ifndef GENINPUT_HID_META_IN_H
#define GENINPUT_HID_META_IN_H

#include <hidsdi.h>
#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "geninput/hid-report-in.h"
#include "geninput/hid.h"

struct hid_control_in {
    struct hid_report_in *report;
    uint16_t collection_id;
};

struct hid_meta_in {
    PHIDP_PREPARSED_DATA ppd;
    HIDP_CAPS caps_tlc;
    HIDP_BUTTON_CAPS *caps_btn;
    HIDP_VALUE_CAPS *caps_val;

    struct hid_control *controls;
    size_t ncontrols;
    size_t nbuttons;

    bool reports_muxed;
    struct hid_control_in *priv_controls;
    struct hid_report_in *reports;
    uint8_t nreports;
};

bool hid_meta_in_init(struct hid_meta_in *meta, PHIDP_PREPARSED_DATA ppd);
bool hid_meta_in_dispatch(
    struct hid_meta_in *meta, uint8_t **bytes, size_t nbytes);
size_t hid_meta_in_get_buffer_size(const struct hid_meta_in *meta);
const struct hid_control *
hid_meta_in_get_controls(const struct hid_meta_in *meta);
size_t hid_meta_in_get_ncontrols(const struct hid_meta_in *meta);
uint32_t hid_meta_in_get_tlc_usage(const struct hid_meta_in *meta);
bool hid_meta_in_get_value(
    struct hid_meta_in *meta, size_t control_no, int32_t *value);
void hid_meta_in_fini(struct hid_meta_in *meta);

#endif
