#define LOG_MODULE "hid-generic"

#include <windows.h>
#include <hidsdi.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "geninput/hid-meta-out.h"
#include "geninput/hid-report-out.h"

#include "util/log.h"
#include "util/mem.h"

static bool hid_meta_out_init_caps(struct hid_meta_out *meta,
        PHIDP_PREPARSED_DATA ppd);
static bool hid_meta_out_init_arrays(struct hid_meta_out *meta);
static void hid_meta_out_init_lights(struct hid_meta_out *meta, HANDLE fd);
static struct hid_report_out *hid_meta_out_lookup_report(
        struct hid_meta_out *meta, uint8_t report_id);
static void hid_meta_out_fini_arrays(struct hid_meta_out *meta);
static void hid_meta_out_fini_caps(struct hid_meta_out *meta);

bool hid_meta_out_init(struct hid_meta_out *meta, PHIDP_PREPARSED_DATA ppd, HANDLE fd)
{
    if (!hid_meta_out_init_caps(meta, ppd)) {
        goto caps_fail;
    }

    if (!hid_meta_out_init_arrays(meta)) {
        goto arrays_fail;
    }

    hid_meta_out_init_lights(meta, fd);

    meta->next_report = 0;

    return true;

arrays_fail:
    hid_meta_out_fini_caps(meta);

caps_fail:
    return false;
}

static bool hid_meta_out_init_caps(struct hid_meta_out *meta,
        PHIDP_PREPARSED_DATA ppd)
{
    uint16_t len;
    NTSTATUS status;

    meta->ppd = ppd;

    status = HidP_GetCaps(meta->ppd, &meta->caps_tlc);

    if (status != HIDP_STATUS_SUCCESS) {
        log_warning("Error getting top-level collection caps");

        goto caps_tlc_fail;
    }

    len = meta->caps_tlc.NumberOutputButtonCaps;
    meta->caps_btn = xmalloc(sizeof(*meta->caps_btn) * len);

    if (len > 0) {
        status = HidP_GetButtonCaps(HidP_Output, meta->caps_btn, &len,
                meta->ppd);

        if (status != HIDP_STATUS_SUCCESS) {
            log_warning("Error getting button caps");

            goto caps_btn_fail;
        }
    }

    len = meta->caps_tlc.NumberOutputValueCaps;
    meta->caps_val = xmalloc(sizeof(*meta->caps_val) * len);

    if (len > 0) {
        status = HidP_GetValueCaps(HidP_Output, meta->caps_val, &len,
                meta->ppd);

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

static bool hid_meta_out_init_arrays(struct hid_meta_out *meta)
{
    const HIDP_BUTTON_CAPS *bc;
    const HIDP_VALUE_CAPS *vc;
    bool *report_presence;
    unsigned int nreports;
    unsigned int i;
    uint8_t *bytes;
    size_t nbytes;
    size_t count;

    /* N.B. "buttons" here are just one-bit values. You have to use a
       completely different API to manipulate these values even though the HID
       spec and protocol doesn't really make a distinction between them. */

    meta->nlights = 0;
    meta->nbuttons = 0;

    /* Use heap-based working area */

    report_presence = xcalloc(sizeof(*report_presence) * 0x100);

    /* Count up buttons, globally and per-report */

    for (i = 0 ; i < meta->caps_tlc.NumberOutputButtonCaps ; i++) {
        bc = &meta->caps_btn[i];

        if (bc->IsRange) {
            count = bc->Range.UsageMax - bc->Range.UsageMin + 1;
        } else {
            count = 1;
        }

        meta->nlights += count;
        meta->nbuttons += count;

        report_presence[bc->ReportID] = true;
    }

    for (i = 0 ; i < meta->caps_tlc.NumberOutputValueCaps ; i++) {
        vc = &meta->caps_val[i];

        if (vc->IsRange) {
            meta->nlights += vc->Range.UsageMax - vc->Range.UsageMin + 1;
        } else {
            meta->nlights += 1;
        }

        report_presence[vc->ReportID] = true;
    }

    /* Count up total number of reports and initialize them */

    nreports = 0;           /* Allocated */
    meta->nreports = 0;     /* Initialized so far (see unwind below) */

    for (i = 0 ; i < 0x100 ; i++) {
        if (report_presence[i]) {
            nreports++;
        }
    }

    meta->reports = xmalloc(sizeof(*meta->reports) * nreports);

    for (i = 0 ; i < 0x100 ; i++) {
        if (report_presence[i]) {
            nbytes = meta->caps_tlc.OutputReportByteLength;
            bytes = xmalloc(meta->caps_tlc.OutputReportByteLength);

            if (!hid_report_out_init(&meta->reports[meta->nreports],
                    meta->ppd, (uint8_t) i, nbytes)) {
                goto r_init_fail;
            }

            meta->nreports++;
        }
    }

    meta->lights = xcalloc(sizeof(*meta->lights) * meta->nlights);
    meta->priv_lights = xmalloc(sizeof(*meta->priv_lights) * meta->nlights);

    /* Clean up our scratch space */

    free(report_presence);

    return true;

r_init_fail:
    free(bytes);

    for (i = meta->nreports ; i > 0 ; i--) {
        hid_report_out_fini(&meta->reports[i - 1]);
    }

    free(report_presence);
    free(meta->reports);

    return false;
}

static void hid_meta_out_init_lights(struct hid_meta_out *meta, HANDLE fd)
{
    const HIDP_BUTTON_CAPS *bc;
    const HIDP_VALUE_CAPS *vc;
    struct hid_out_light *priv;
    struct hid_light *light;
    unsigned int pos;
    unsigned int i;
    int j;

    pos = 0;

    for (i = 0 ; i < meta->caps_tlc.NumberOutputButtonCaps ; i++) {
        bc = &meta->caps_btn[i];

        if (bc->IsRange) {
            for (j = 0 ; j <= bc->Range.UsageMax - bc->Range.UsageMin ; j++) {
                priv = &meta->priv_lights[pos];

                priv->report = hid_meta_out_lookup_report(meta, bc->ReportID);
                priv->collection_id = bc->LinkCollection;
                priv->size = 1;

                light = &meta->lights[pos];

                light->usage = (bc->UsagePage << 16) | (bc->Range.UsageMin + j);
                light->value_min = 0;
                light->value_max = 1;

                // Devices may specify their own strings for lights
                if(bc->IsStringRange && bc->Range.StringMin != 0) {
                    HidD_GetIndexedString(fd, bc->Range.StringMin + j, light->name, sizeof(light->name));
                } else if(!bc->IsStringRange && bc->NotRange.StringIndex != 0) {
                    HidD_GetIndexedString(fd, bc->NotRange.StringIndex, light->name, sizeof(light->name));
                }

                pos++;
            }
        } else {
            priv = &meta->priv_lights[pos];

            priv->report = hid_meta_out_lookup_report(meta, bc->ReportID);
            priv->collection_id = bc->LinkCollection;
            priv->size = 1;

            light = &meta->lights[pos];

            light->usage = (bc->UsagePage << 16) | bc->NotRange.Usage;
            light->value_min = 0;
            light->value_max = 1;

            if(!bc->IsStringRange && bc->NotRange.StringIndex != 0) {
                HidD_GetIndexedString(fd, bc->NotRange.StringIndex, light->name, sizeof(light->name));
            }

            pos++;
        }
    }

    for (i = 0 ; i < meta->caps_tlc.NumberOutputValueCaps ; i++) {
        vc = &meta->caps_val[i];

        if (vc->IsRange) {
            for (j = 0 ; j <= vc->Range.UsageMax - vc->Range.UsageMin ; j++) {
                priv = &meta->priv_lights[pos];

                priv->report = hid_meta_out_lookup_report(meta, vc->ReportID);
                priv->collection_id = vc->LinkCollection;
                priv->size = vc->BitSize;

                light = &meta->lights[pos];

                light->usage = (vc->UsagePage << 16) | (vc->Range.UsageMin + j);
                light->value_min = vc->LogicalMin;
                light->value_max = vc->LogicalMax;

                // Devices may specify their own strings for lights
                if(vc->IsStringRange && vc->Range.StringMin != 0) {
                    HidD_GetIndexedString(fd, vc->Range.StringMin + j, light->name, sizeof(light->name));
                } else if(!vc->IsStringRange && vc->NotRange.StringIndex != 0) {
                    HidD_GetIndexedString(fd, vc->NotRange.StringIndex, light->name, sizeof(light->name));
                }

                pos++;
            }
        } else {
            priv = &meta->priv_lights[pos];

            priv->report = hid_meta_out_lookup_report(meta, vc->ReportID);
            priv->collection_id = vc->LinkCollection;
            priv->size = vc->BitSize;

            light = &meta->lights[pos];

            light->usage = (vc->UsagePage << 16) | vc->NotRange.Usage;
            light->value_min = vc->LogicalMin;
            light->value_max = vc->LogicalMax;

            if(!vc->IsStringRange && vc->NotRange.StringIndex != 0) {
                HidD_GetIndexedString(fd, vc->NotRange.StringIndex, light->name, sizeof(light->name));
            }

            pos++;
        }
    }
}

static struct hid_report_out *hid_meta_out_lookup_report(
        struct hid_meta_out *meta, uint8_t report_id)
{
    unsigned int i;

    for (i = 0 ; i < meta->nreports ; i++) {
        if (meta->reports[i].id == report_id) {
            return &meta->reports[i];
        }
    }

    return NULL;
}

size_t hid_meta_out_get_buffer_size(const struct hid_meta_out *meta)
{
    return meta->caps_tlc.OutputReportByteLength;
}

const struct hid_light *hid_meta_out_get_lights(
        const struct hid_meta_out *meta)
{
    return meta->lights;
}

size_t hid_meta_out_get_nlights(const struct hid_meta_out *meta)
{
    return meta->nlights;
}

bool hid_meta_out_set_light(struct hid_meta_out *meta,
        size_t light_no, uint32_t intensity)
{
    struct hid_out_light *priv;
    uint32_t usage;

    if (light_no >= meta->nlights) {
        return false;
    }

    priv = &meta->priv_lights[light_no];
    usage = meta->lights[light_no].usage;

    if (light_no < meta->nbuttons) {
        return hid_report_out_set_bit(priv->report, meta->ppd,
                priv->collection_id, usage, intensity != 0);
    } else {
        return hid_report_out_set_value(priv->report, meta->ppd,
                priv->collection_id, usage, intensity);
    }
}

void hid_meta_out_get_next_report(struct hid_meta_out *meta, uint8_t *bytes)
{
    struct hid_report_out *r;

    log_assert(meta->nreports > 0);

    r = &meta->reports[meta->next_report];
    meta->next_report = (meta->next_report + 1) % meta->nreports;

    hid_report_out_get_bytes(r, bytes);
}

void hid_meta_out_fini(struct hid_meta_out *meta)
{
    hid_meta_out_fini_arrays(meta);
    hid_meta_out_fini_caps(meta);
}

static void hid_meta_out_fini_arrays(struct hid_meta_out *meta)
{
    unsigned int i;

    free(meta->priv_lights);
    free(meta->lights);

    for (i = 0 ; i < meta->nreports ; i++) {
        hid_report_out_fini(&meta->reports[i]);
    }

    free(meta->reports);
}

static void hid_meta_out_fini_caps(struct hid_meta_out *meta)
{
    free(meta->caps_btn);
    free(meta->caps_val);
}

