#define LOG_MODULE "pacdrive"

#include <hidsdi.h>
#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "geninput/hid.h"
#include "geninput/io-thread.h"
#include "geninput/pacdrive.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"

/* The PacDrive appears to have a malformed descriptor for its OUT report.
   Since PacDrives are fairly prevalent, we have to have a special-case HID
   driver for them that uses a hard-coded report structure. */

struct pac_report {
    uint8_t report_id;
    uint8_t static_00;
    uint8_t static_dd;
    uint8_t leds_hi;
    uint8_t leds_lo;
};

struct pac {
    struct hid_fd super;
    HANDLE fd;
    OVERLAPPED ovl;
    struct pac_report io_buf;
    uint16_t leds;
};

static struct hid_light pac_lights[16];
static const wchar_t pac_name[] = L"PacDrive Shim";

static bool pac_get_device_usage(const struct hid *hid, uint32_t *usage);
static bool pac_get_name(const struct hid *hid, wchar_t *chars, size_t *nchars);
static bool pac_get_controls(
    const struct hid *hid, struct hid_control *controls, size_t *ncontrols);
static bool pac_get_lights(
    const struct hid *hid, struct hid_light *lights, size_t *nlights);
static bool pac_get_value(struct hid *hid, size_t control_no, int32_t *value);
static bool pac_set_light(struct hid *hid, size_t light_no, uint32_t intensity);
static bool
pac_handle_event(struct hid_fd *hid_fd, OVERLAPPED *ovl, size_t nbytes);
static void pac_close(struct hid *hid);

static const struct hid_fd_vtbl pac_vtbl = {
    {/* .close               = */ pac_close,
     /* .get_device_usage    = */ pac_get_device_usage,
     /* .get_name            = */ pac_get_name,
     /* .get_controls        = */ pac_get_controls,
     /* .get_lights          = */ pac_get_lights,
     /* .get_value           = */ pac_get_value,
     /* .set_light           = */ pac_set_light},
    /* .handle_event        = */ pac_handle_event};

bool pac_open(
    struct hid_fd **hid_out,
    const char *dev_node,
    HANDLE iocp,
    uintptr_t iocp_ctx)
{
    HIDD_ATTRIBUTES attrs;
    HANDLE fd;
    struct pac *pac;
    int i;

    fd = CreateFile(
        dev_node,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (fd == INVALID_HANDLE_VALUE) {
        goto open_fail;
    }

    if (!HidD_GetAttributes(fd, &attrs)) {
        goto attrs_fail;
    }

    if (attrs.VendorID != 0xD209 || (attrs.ProductID & 0xFFF8) != 0x1500) {
        goto id_fail;
    }

    pac = xcalloc(sizeof(*pac));

    pac->super.vptr = &pac_vtbl;
    pac->fd = fd;

    CreateIoCompletionPort(fd, iocp, iocp_ctx, 0);

    if (!WriteFile(fd, &pac->io_buf, sizeof(pac->io_buf), NULL, &pac->ovl) &&
        GetLastError() != ERROR_IO_PENDING) {
        log_warning(
            "%s: Initial WriteFile failed: %08x",
            dev_node,
            (unsigned int) GetLastError());

        goto write_fail;
    }

    for (i = 0; i < lengthof(pac_lights); i++) {
        pac_lights[i].usage = 0x0008004B; /* LED, Generic Indicator */
        pac_lights[i].value_min = 0;
        pac_lights[i].value_max = 1;
    }

    log_misc("Opened PacDrive on %s", dev_node);

    *hid_out = &pac->super;

    return true;

write_fail:
    free(pac);

id_fail:
attrs_fail:
    CloseHandle(fd);

open_fail:
    return false;
}

static bool pac_get_device_usage(const struct hid *hid, uint32_t *usage)
{
    *usage = 0x00010000;

    return true;
}

static bool pac_get_name(const struct hid *hid, wchar_t *chars, size_t *nchars)
{
    log_assert(nchars != NULL);

    if (chars != NULL) {
        if (*nchars < lengthof(pac_name)) {
            return false;
        }

        memcpy(chars, pac_name, sizeof(pac_name));
    }

    *nchars = lengthof(pac_name);

    return true;
}

static bool pac_get_controls(
    const struct hid *hid, struct hid_control *controls, size_t *ncontrols)
{
    log_assert(ncontrols != NULL);

    *ncontrols = 0;

    return true;
}

static bool
pac_get_lights(const struct hid *hid, struct hid_light *lights, size_t *nlights)
{
    log_assert(nlights != NULL);

    if (lights != NULL) {
        if (*nlights < lengthof(pac_lights)) {
            return false;
        }

        memcpy(lights, pac_lights, sizeof(pac_lights));
    }

    *nlights = lengthof(pac_lights);

    return true;
}

static bool pac_get_value(struct hid *hid, size_t control_no, int32_t *value)
{
    return false;
}

static bool pac_set_light(struct hid *hid, size_t light_no, uint32_t intensity)
{
    struct pac *pac = containerof(hid, struct pac, super);

    if (light_no >= lengthof(pac_lights)) {
        return false;
    }

    if (intensity != 0) {
        pac->leds |= (1 << light_no);
    } else {
        pac->leds &= ~(1 << light_no);
    }

    return true;
}

static bool
pac_handle_event(struct hid_fd *hid_fd, OVERLAPPED *ovl, size_t nbytes)
{
    struct pac *pac = containerof(hid_fd, struct pac, super);

    /* OUT report successfully sent, send another */

    pac->io_buf.leds_hi = HIBYTE(pac->leds);
    pac->io_buf.leds_lo = LOBYTE(pac->leds);

    memset(&pac->ovl, 0, sizeof(pac->ovl));

    if (!WriteFile(
            pac->fd, &pac->io_buf, sizeof(pac->io_buf), NULL, &pac->ovl) &&
        GetLastError() != ERROR_IO_PENDING) {
        log_warning("WriteFile failed: %08x", (unsigned int) GetLastError());

        return false;
    }

    return true;
}

static void pac_close(struct hid *hid)
{
    struct pac *pac = containerof(hid, struct pac, super);

    CloseHandle(pac->fd);

    free(pac);
}
