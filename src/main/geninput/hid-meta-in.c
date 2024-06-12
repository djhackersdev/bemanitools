#define LOG_MODULE "hid-generic"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <hidsdi.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "geninput/hid-meta-in.h"
#include "geninput/hid-report-in.h"

#include "iface-core/log.h"

#include "util/mem.h"

static bool
hid_meta_in_init_caps(struct hid_meta_in *meta, PHIDP_PREPARSED_DATA ppd);
static void hid_meta_in_init_arrays(struct hid_meta_in *meta);
static void hid_meta_in_init_buttons(struct hid_meta_in *meta);
static void hid_meta_in_init_values(struct hid_meta_in *meta);
static struct hid_report_in *
hid_meta_in_lookup_report(struct hid_meta_in *meta, uint8_t report_id);
static void hid_meta_in_fini_arrays(struct hid_meta_in *meta);
static void hid_meta_in_fini_caps(struct hid_meta_in *meta);

bool hid_meta_in_init(struct hid_meta_in *meta, PHIDP_PREPARSED_DATA ppd)
{
    if (!hid_meta_in_init_caps(meta, ppd)) {
        return false;
    }

    hid_meta_in_init_arrays(meta);
    hid_meta_in_init_buttons(meta);
    hid_meta_in_init_values(meta);

    return true;
}

static bool
hid_meta_in_init_caps(struct hid_meta_in *meta, PHIDP_PREPARSED_DATA ppd)
{
    uint16_t len;
    NTSTATUS status;

    meta->ppd = ppd;

    status = HidP_GetCaps(meta->ppd, &meta->caps_tlc);

    if (status != HIDP_STATUS_SUCCESS) {
        log_warning("Error getting top-level collection caps");

        goto caps_tlc_fail;
    }

    len = meta->caps_tlc.NumberInputButtonCaps;
    meta->caps_btn = xmalloc(sizeof(*meta->caps_btn) * len);

    if (len > 0) {
        status =
            HidP_GetButtonCaps(HidP_Input, meta->caps_btn, &len, meta->ppd);

        if (status != HIDP_STATUS_SUCCESS) {
            log_warning("Error getting button caps");

            goto caps_btn_fail;
        }
    }

    len = meta->caps_tlc.NumberInputValueCaps;
    meta->caps_val = xmalloc(sizeof(*meta->caps_val) * len);

    if (len > 0) {
        status = HidP_GetValueCaps(HidP_Input, meta->caps_val, &len, meta->ppd);

        if (status != HIDP_STATUS_SUCCESS) {
            log_warning("Error getting value caps");

            goto caps_val_fail;
        }
    }

    return true;

caps_val_fail:
    free(meta->caps_val);

caps_btn_fail:
    free(meta->caps_btn);

caps_tlc_fail:
    return false;
}

static void hid_meta_in_init_arrays(struct hid_meta_in *meta)
{
    const HIDP_BUTTON_CAPS *bc;
    const HIDP_VALUE_CAPS *vc;
    uint16_t *report_btns;
    bool *report_presence;
    uint16_t count;
    unsigned int i;
    unsigned int j;
    void *report_buf;

    /* Init counters */

    meta->ncontrols = 0;
    meta->nbuttons = 0;
    meta->nreports = 0;

    meta->reports_muxed = false;

    /* We have extremely limited stack headroom when called from IIDX for some
       reason, gotta do this on the heap... */

    report_btns = xcalloc(sizeof(*report_btns) * 0x100);
    report_presence = xcalloc(sizeof(*report_presence) * 0x100);

    /* Count up buttons, globally and per-report */

    for (i = 0; i < meta->caps_tlc.NumberInputButtonCaps; i++) {
        bc = &meta->caps_btn[i];

        if (bc->IsRange) {
            count = bc->Range.UsageMax - bc->Range.UsageMin + 1;
        } else {
            count = 1;
        }

        if (bc->ReportID != 0) {
            meta->reports_muxed = true;
        }

        report_btns[bc->ReportID] += count;

        meta->ncontrols += count;
        meta->nbuttons += count;
    }

    /* Count up values, note all reports that have values */

    for (i = 0; i < meta->caps_tlc.NumberInputValueCaps; i++) {
        vc = &meta->caps_val[i];

        if (vc->IsRange) {
            count = vc->Range.UsageMax - vc->Range.UsageMin + 1;
        } else {
            count = 1;
        }

        if (vc->ReportID != 0) {
            meta->reports_muxed = true;
        }

        report_presence[vc->ReportID] = true;

        meta->ncontrols += count;
    }

    /* Count up total number of reports and initialize them */

    for (i = 0; i < 0x100; i++) {
        if (report_btns[i] || report_presence[i]) {
            meta->nreports++;
        }
    }

    j = 0;
    meta->reports = xmalloc(sizeof(*meta->reports) * meta->nreports);

    for (i = 0; i < 0x100; i++) {
        if (report_btns[i] || report_presence[i]) {
            report_buf = xmalloc(meta->caps_tlc.InputReportByteLength);

            hid_report_in_init(
                &meta->reports[j++],
                (uint8_t) i,
                report_btns[i],
                report_buf,
                meta->caps_tlc.InputReportByteLength,
                meta->ppd);
        }
    }

    /* Init control arrays */

    meta->controls = xmalloc(sizeof(*meta->controls) * meta->ncontrols);
    meta->priv_controls =
        xmalloc(sizeof(*meta->priv_controls) * meta->ncontrols);

    /* Clean up our scratch space */

    free(report_presence);
    free(report_btns);
}

static void hid_meta_in_init_buttons(struct hid_meta_in *meta)
{
    const HIDP_BUTTON_CAPS *bc;
    struct hid_control_in *priv;
    struct hid_control *ctl;
    unsigned int pos;
    unsigned int i;
    int j;

    pos = 0;

    for (i = 0; i < meta->caps_tlc.NumberInputButtonCaps; i++) {
        bc = &meta->caps_btn[i];

        if (bc->IsRange) {
            for (j = 0; j <= bc->Range.UsageMax - bc->Range.UsageMin; j++) {
                priv = &meta->priv_controls[pos];

                priv->report = hid_meta_in_lookup_report(meta, bc->ReportID);
                priv->collection_id = bc->LinkCollection;

                ctl = &meta->controls[pos];

                ctl->usage = (bc->UsagePage << 16) | (bc->Range.UsageMin + j);
                ctl->value_min = 0;
                ctl->value_max = 1;
                ctl->flags = 0;

                pos++;
            }
        } else {
            priv = &meta->priv_controls[pos];

            priv->report = hid_meta_in_lookup_report(meta, bc->ReportID);
            priv->collection_id = bc->LinkCollection;

            ctl = &meta->controls[pos];

            ctl->usage = (bc->UsagePage << 16) | bc->NotRange.Usage;
            ctl->value_min = 0;
            ctl->value_max = 1;
            ctl->flags = 0;

            pos++;
        }
    }
}

static void hid_meta_in_init_values(struct hid_meta_in *meta)
{
    const HIDP_VALUE_CAPS *vc;
    struct hid_control_in *priv;
    struct hid_control *ctl;
    unsigned int pos;
    unsigned int i;
    int j;

    pos = meta->nbuttons;

    for (i = 0; i < meta->caps_tlc.NumberInputValueCaps; i++) {
        vc = &meta->caps_val[i];

        if (vc->IsRange) {
            for (j = 0; j <= vc->Range.UsageMax - vc->Range.UsageMin; j++) {
                priv = &meta->priv_controls[pos];

                priv->report = hid_meta_in_lookup_report(meta, vc->ReportID);
                priv->collection_id = vc->LinkCollection;

                ctl = &meta->controls[pos];

                ctl->usage = (vc->UsagePage << 16) | (vc->Range.UsageMin + i);
                ctl->value_min = vc->LogicalMin;
                ctl->value_max = vc->LogicalMax;
                ctl->flags = 0;

                if (vc->HasNull) {
                    ctl->flags |= HID_FLAG_NULLABLE;
                }

                if (!vc->IsAbsolute) {
                    ctl->flags |= HID_FLAG_RELATIVE;
                }

                pos++;
            }
        } else {
            priv = &meta->priv_controls[pos];

            priv->report = hid_meta_in_lookup_report(meta, vc->ReportID);
            priv->collection_id = vc->LinkCollection;

            ctl = &meta->controls[pos];

            ctl->usage = (vc->UsagePage << 16) | vc->NotRange.Usage;
            ctl->value_min = vc->LogicalMin;
            ctl->value_max = vc->LogicalMax;
            ctl->flags = 0;

            if (vc->HasNull) {
                ctl->flags |= HID_FLAG_NULLABLE;
            }

            if (!vc->IsAbsolute) {
                ctl->flags |= HID_FLAG_RELATIVE;
            }

            pos++;
        }
    }
}

static struct hid_report_in *
hid_meta_in_lookup_report(struct hid_meta_in *meta, uint8_t report_id)
{
    unsigned int i;

    for (i = 0; i < meta->nreports; i++) {
        if (meta->reports[i].id == report_id) {
            return &meta->reports[i];
        }
    }

    return NULL;
}

bool hid_meta_in_dispatch(
    struct hid_meta_in *meta, uint8_t **bytes, size_t nbytes)
{
    struct hid_report_in *r;
    uint8_t report_id;

    if (meta->reports_muxed) {
        report_id = **bytes;
    } else {
        report_id = 0;
    }

    r = hid_meta_in_lookup_report(meta, report_id);

    if (r == NULL) {
        log_warning("Got spurious report with ID %02x", report_id);

        return false;
    }

    return hid_report_in_exchange(r, bytes, nbytes);
}

size_t hid_meta_in_get_buffer_size(const struct hid_meta_in *meta)
{
    return meta->caps_tlc.InputReportByteLength;
}

const struct hid_control *
hid_meta_in_get_controls(const struct hid_meta_in *meta)
{
    return meta->controls;
}

size_t hid_meta_in_get_ncontrols(const struct hid_meta_in *meta)
{
    return meta->ncontrols;
}

uint32_t hid_meta_in_get_tlc_usage(const struct hid_meta_in *meta)
{
    return (meta->caps_tlc.UsagePage << 16) | meta->caps_tlc.Usage;
}

bool hid_meta_in_get_value(
    struct hid_meta_in *meta, size_t control_no, int32_t *value)
{
    struct hid_control_in *priv;
    uint32_t usage;

    if (control_no >= meta->ncontrols) {
        return false;
    }

    priv = &meta->priv_controls[control_no];
    usage = meta->controls[control_no].usage;

    if (control_no < meta->nbuttons) {
        return hid_report_in_get_bit(
            priv->report, meta->ppd, priv->collection_id, usage, value);
    } else {
        return hid_report_in_get_value(
            priv->report, meta->ppd, priv->collection_id, usage, value);
    }
}

void hid_meta_in_fini(struct hid_meta_in *meta)
{
    hid_meta_in_fini_arrays(meta);
    hid_meta_in_fini_caps(meta);
}

static void hid_meta_in_fini_arrays(struct hid_meta_in *meta)
{
    unsigned int i;

    free(meta->priv_controls);
    free(meta->controls);

    for (i = 0; i < meta->nreports; i++) {
        hid_report_in_fini(&meta->reports[i]);
    }

    free(meta->reports);
}

static void hid_meta_in_fini_caps(struct hid_meta_in *meta)
{
    free(meta->caps_val);
    free(meta->caps_btn);
}
