#define LOG_MODULE "hid-generic"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <hidsdi.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "geninput/hid-report-in.h"

#include "iface-core/log.h"

#include "util/mem.h"

void hid_report_in_init(
    struct hid_report_in *r,
    uint8_t report_id,
    uint32_t nbuttons,
    void *bytes,
    size_t nbytes,
    PHIDP_PREPARSED_DATA ppd)
{
    r->id = report_id;
    r->nbuttons = nbuttons;
    r->buttons = xmalloc(sizeof(*r->buttons) * r->nbuttons);
    r->bytes = bytes;
    r->nbytes = nbytes;

    if (HidP_InitializeReportForID(HidP_Input, report_id, ppd, bytes, nbytes) !=
        HIDP_STATUS_SUCCESS) {
        log_warning("Error initializing IN report %02X", report_id);
    }
}

uint8_t hid_report_in_get_id(struct hid_report_in *r)
{
    return r->id;
}

bool hid_report_in_exchange(
    struct hid_report_in *r, uint8_t **bytes, uint32_t nbytes)
{
    uint8_t *last_bytes;

    last_bytes = r->bytes;

    r->bytes = *bytes;
    r->nbytes = nbytes;

    *bytes = last_bytes;

    return true;
}

bool hid_report_in_get_bit(
    struct hid_report_in *r,
    PHIDP_PREPARSED_DATA ppd,
    uint16_t collection_id,
    uint32_t usage,
    int32_t *value)
{
    unsigned long i;
    unsigned long nbuttons;
    NTSTATUS result;

    nbuttons = r->nbuttons;
    result = HidP_GetUsagesEx(
        HidP_Input,
        collection_id,
        (USAGE_AND_PAGE *) r->buttons,
        &nbuttons,
        ppd,
        (PCHAR) r->bytes,
        (unsigned long) r->nbytes);

    if (result != HIDP_STATUS_SUCCESS) {
        log_warning(
            "HidP_GetUsagesEx failed for report %02X: %08x",
            r->id,
            (unsigned int) result);

        return false;
    }

    *value = 0;

    for (i = 0; i < nbuttons; i++) {
        if (r->buttons[i] == usage) {
            *value = 1;
        }
    }

    return true;
}

bool hid_report_in_get_value(
    struct hid_report_in *r,
    PHIDP_PREPARSED_DATA ppd,
    uint16_t collection_id,
    uint32_t usage,
    int32_t *value)
{
    NTSTATUS result;

    if (r->bytes == NULL) {
        log_warning("Report %02X not yet valid", r->id);

        return false;
    }

    result = HidP_GetUsageValue(
        HidP_Input,
        usage >> 16,
        0,
        (uint16_t) usage,
        (unsigned long *) value,
        ppd,
        (PCHAR) r->bytes,
        r->nbytes);

    if (result != HIDP_STATUS_SUCCESS) {
        log_warning(
            "HidP_GetUsageValue(%08x) failed for report %02X: %08x",
            usage,
            r->id,
            (unsigned int) result);

        return false;
    }

    return true;
}

void hid_report_in_fini(struct hid_report_in *r)
{
    free(r->buttons);
    free(r->bytes);
}
