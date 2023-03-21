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

#include "geninput/hid-report-out.h"

#include "util/log.h"
#include "util/mem.h"

bool hid_report_out_init(
    struct hid_report_out *r,
    PHIDP_PREPARSED_DATA ppd,
    uint8_t id,
    size_t nbytes)
{
    NTSTATUS status;

    r->id = id;
    r->nbytes = nbytes;
    r->bytes = xmalloc(nbytes);

    status = HidP_InitializeReportForID(
        HidP_Output, r->id, ppd, (PCHAR) r->bytes, r->nbytes);

    if (status != HIDP_STATUS_SUCCESS) {
        log_warning(
            "Report %02X: HidP_InitializeReportForID failed: %08x",
            id,
            (unsigned int) status);

        goto fail;
    }

    return true;

fail:
    free(r->bytes);

    return false;
}

void hid_report_out_get_bytes(const struct hid_report_out *r, uint8_t *bytes)
{
    memcpy(bytes, r->bytes, r->nbytes);
}

bool hid_report_out_set_bit(
    struct hid_report_out *r,
    PHIDP_PREPARSED_DATA ppd,
    uint16_t collection_id,
    uint32_t usage,
    bool value)
{
    NTSTATUS status;
    unsigned long count;
    uint16_t usage_hi;
    uint16_t usage_lo;

    usage_hi = (uint16_t) (usage >> 16);
    usage_lo = (uint16_t) (usage >> 0);
    count = 1;

    if (value) {
        status = HidP_SetUsages(
            HidP_Output,
            usage_hi,
            collection_id,
            &usage_lo,
            &count,
            ppd,
            (PCHAR) r->bytes,
            r->nbytes);

        if (status != HIDP_STATUS_SUCCESS) {
            log_warning("HidP_SetUsages failed: %08x", (unsigned int) status);

            return false;
        }
    } else {
        status = HidP_UnsetUsages(
            HidP_Output,
            usage_hi,
            collection_id,
            &usage_lo,
            &count,
            ppd,
            (PCHAR) r->bytes,
            r->nbytes);

        if (status != HIDP_STATUS_SUCCESS &&
            status != HIDP_STATUS_BUTTON_NOT_PRESSED /* jfc */) {
            log_warning("HidP_UnsetUsages failed: %08x", (unsigned int) status);

            return false;
        }
    }

    return true;
}

bool hid_report_out_set_value(
    struct hid_report_out *r,
    PHIDP_PREPARSED_DATA ppd,
    uint16_t collection_id,
    uint32_t usage,
    uint32_t value)
{
    NTSTATUS status;
    uint16_t usage_hi;
    uint16_t usage_lo;

    usage_hi = (uint16_t) (usage >> 16);
    usage_lo = (uint16_t) (usage >> 0);

    status = HidP_SetUsageValue(
        HidP_Output,
        usage_hi,
        collection_id,
        usage_lo,
        value,
        ppd,
        (PCHAR) r->bytes,
        r->nbytes);

    if (status != HIDP_STATUS_SUCCESS) {
        log_warning("HidP_SetUsages failed: %08x", (unsigned int) status);

        return false;
    }

    return true;
}

void hid_report_out_fini(struct hid_report_out *r)
{
    free(r->bytes);
}
