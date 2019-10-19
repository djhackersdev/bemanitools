#define LOG_MODULE "hid-generic"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <hidsdi.h>
#include <windows.h>

#include "geninput/hid-generic-strings.h"
#include "geninput/hid-generic.h"
#include "geninput/hid-meta-in.h"
#include "geninput/hid-meta-out.h"
#include "geninput/hid.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"

struct hid_generic {
    struct hid_fd super;

    char *dev_node;

    HANDLE fd;
    PHIDP_PREPARSED_DATA ppd;

    OVERLAPPED in_ovl;
    OVERLAPPED out_ovl;

    struct hid_generic_strings strings;
    struct hid_meta_in meta_in;
    struct hid_meta_out meta_out;

    uint8_t *in_buf;
    size_t in_nbytes;

    uint8_t *out_buf;
    size_t out_nbytes;
    bool out_started;
    bool out_faulty;
};

static bool
hid_generic_get_device_usage(const struct hid *hid, uint32_t *usage);
static bool
hid_generic_get_name(const struct hid *hid, wchar_t *chars, size_t *nchars);
static bool hid_generic_get_controls(
    const struct hid *hid, struct hid_control *controls, size_t *ncontrols);
static bool hid_generic_get_lights(
    const struct hid *hid, struct hid_light *lights, size_t *nlights);
static bool
hid_generic_get_value(struct hid *hid, size_t control_no, int32_t *out_value);
static bool
hid_generic_set_light(struct hid *hid, size_t light_no, uint32_t intensity);
static bool
hid_generic_handle_event(struct hid_fd *super, OVERLAPPED *ovl, size_t nbytes);
static void hid_generic_close(struct hid *hid);

static struct hid_fd_vtbl hid_generic_vtbl = {
    {/* .close               = */ hid_generic_close,
     /* .get_device_usage    = */ hid_generic_get_device_usage,
     /* .get_name            = */ hid_generic_get_name,
     /* .get_controls        = */ hid_generic_get_controls,
     /* .get_lights          = */ hid_generic_get_lights,
     /* .get_value           = */ hid_generic_get_value,
     /* .set_light           = */ hid_generic_set_light},
    /* .handle_event        = */ hid_generic_handle_event};

bool hid_generic_open(
    struct hid_fd **out, const char *dev_node, HANDLE iocp, uintptr_t iocp_ctx)
{
    struct hid_generic *hg;
    uint32_t tlc_usage;

    hg = xmalloc(sizeof(*hg));

    hg->fd = CreateFile(
        dev_node,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (hg->fd == INVALID_HANDLE_VALUE) {
        /* Probably a keyboard that's locked by the OS */
        goto open_fail;
    }

    if (!HidD_GetPreparsedData(hg->fd, &hg->ppd)) {
        log_warning("HidD_GetPreparsedData failed on dev node %s", dev_node);

        goto ppd_fail;
    }

    if (!hid_meta_in_init(&hg->meta_in, hg->ppd)) {
        goto meta_in_fail;
    }

    tlc_usage = hid_meta_in_get_tlc_usage(&hg->meta_in);

    if ((tlc_usage >> 24) == 0xFF) {
        log_misc("Skipping vendor-specific HID on dev node %s", dev_node);

        goto vsp_fail;
    }

    if (!hid_meta_out_init(&hg->meta_out, hg->ppd, hg->fd)) {
        goto meta_out_fail;
    }

    hid_generic_strings_init(&hg->strings, hg->fd);

    /* Dump diagnostics */

    log_misc("Initializing generic HID on dev node %s:", dev_node);
    log_misc("... Product: %s", hg->strings.str_prod);
    log_misc("... Manufacturer: %s", hg->strings.str_manf);
    log_misc("... Device usage: %08x", hid_meta_in_get_tlc_usage(&hg->meta_in));
    log_misc(
        "... Number of controls: %u",
        (unsigned int) hid_meta_in_get_ncontrols(&hg->meta_in));
    log_misc(
        "... Number of \"lights\": %u",
        (unsigned int) hid_meta_out_get_nlights(&hg->meta_out));

    /* Start up input double-buffer chain by reading a report.
       We bind to the IO completion port here. */

    CreateIoCompletionPort(hg->fd, iocp, iocp_ctx, 0);

    hg->in_nbytes = hid_meta_in_get_buffer_size(&hg->meta_in);
    hg->in_buf = xmalloc(hg->in_nbytes);

    memset(&hg->in_ovl, 0, sizeof(hg->in_ovl));

    if (!ReadFile(hg->fd, hg->in_buf, hg->in_nbytes, NULL, &hg->in_ovl)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            log_warning(
                "Initial ReadFile failed: %08x: %s",
                (unsigned int) GetLastError(),
                dev_node);

            goto read_fail;
        }
    }

    /* Allocate OUT report buffers, but don't start sending the reports until
       the user tries to set a light. Some HIDs (e.g. Razer devices) freak out
       if you send them HID reports that they aren't expecting.

       These OUT reports are probably used for configuration settings or,
       worse yet, firmware updates. Configuration changes are what feature
       reports are for. As for firmware updates over HID... no. Just, no. */

    hg->out_nbytes = hid_meta_out_get_buffer_size(&hg->meta_out);
    hg->out_buf = xmalloc(hg->out_nbytes);
    hg->out_started = false;

    memset(&hg->out_ovl, 0, sizeof(hg->out_ovl));

    /* (finish up and return success) */

    hg->dev_node = str_dup(dev_node);
    hg->super.vptr = &hid_generic_vtbl;

    *out = &hg->super;

    return true;

read_fail:
    free(hg->in_buf);

    hid_generic_strings_fini(&hg->strings);
    hid_meta_out_fini(&hg->meta_out);

meta_out_fail:
vsp_fail:
    hid_meta_in_fini(&hg->meta_in);

meta_in_fail:
    HidD_FreePreparsedData(hg->ppd);

ppd_fail:
    CloseHandle(hg->fd);

open_fail:
    free(hg);

    return false;
}

static bool hid_generic_get_device_usage(const struct hid *hid, uint32_t *usage)
{
    const struct hid_generic *hg = containerof(hid, struct hid_generic, super);

    *usage = hid_meta_in_get_tlc_usage(&hg->meta_in);

    return true;
}

static bool
hid_generic_get_name(const struct hid *hid, wchar_t *chars, size_t *nchars)
{
    const struct hid_generic *hg = containerof(hid, struct hid_generic, super);
    size_t len;

    log_assert(nchars != NULL);

    len = wcslen(hg->strings.wstr_prod) + 1;

    if (chars == NULL) {
        *nchars = len;

        return true;
    } else if (*nchars >= len) {
        memcpy(chars, hg->strings.wstr_prod, len * sizeof(wchar_t));

        return true;
    } else {
        return false;
    }
}

static bool hid_generic_get_controls(
    const struct hid *hid, struct hid_control *controls, size_t *ncontrols)
{
    const struct hid_generic *hg = containerof(hid, struct hid_generic, super);

    const struct hid_control *meta_controls;
    size_t meta_ncontrols;

    if (ncontrols == NULL) {
        return false;
    }

    meta_controls = hid_meta_in_get_controls(&hg->meta_in);
    meta_ncontrols = hid_meta_in_get_ncontrols(&hg->meta_in);

    if (controls == NULL) {
        *ncontrols = meta_ncontrols;

        return true;
    } else if (*ncontrols >= meta_ncontrols) {
        *ncontrols = meta_ncontrols;
        memcpy(controls, meta_controls, sizeof(*controls) * meta_ncontrols);

        return true;
    } else {
        return false;
    }
}

static bool hid_generic_get_lights(
    const struct hid *hid, struct hid_light *lights, size_t *nlights)
{
    const struct hid_generic *hg =
        containerof(hid, const struct hid_generic, super);

    const struct hid_light *meta_lights;
    size_t meta_nlights;

    log_assert(nlights != NULL);

    meta_lights = hid_meta_out_get_lights(&hg->meta_out);
    meta_nlights = hid_meta_out_get_nlights(&hg->meta_out);

    if (lights == NULL) {
        *nlights = meta_nlights;

        return true;
    } else if (*nlights >= meta_nlights) {
        *nlights = meta_nlights;
        memcpy(lights, meta_lights, sizeof(*lights) * meta_nlights);

        return true;
    } else {
        return false;
    }
}

static bool
hid_generic_get_value(struct hid *hid, size_t control_no, int32_t *out_value)
{
    struct hid_generic *hg = containerof(hid, struct hid_generic, super);

    return hid_meta_in_get_value(&hg->meta_in, control_no, out_value);
}

static bool
hid_generic_set_light(struct hid *hid, size_t light_no, uint32_t intensity)
{
    struct hid_generic *hg = containerof(hid, struct hid_generic, super);

    size_t nlights;

    nlights = hid_meta_out_get_nlights(&hg->meta_out);

    if (light_no >= nlights) {
        return false;
    }

    /* Lazily start the lighting double-buffer chain (see hid_generic_init) */

    if (!hg->out_started && !hg->out_faulty) {
        log_misc("Starting light output to %s", hg->dev_node);
        hid_meta_out_get_next_report(&hg->meta_out, hg->out_buf);

        if (!WriteFile(
                hg->fd, hg->out_buf, hg->out_nbytes, NULL, &hg->out_ovl)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                log_warning(
                    "Initial WriteFile failed: %08x: %s",
                    (unsigned int) GetLastError(),
                    hg->dev_node);
                hg->out_faulty = true;

                return false;
            }
        }

        hg->out_started = true;
    }

    return hid_meta_out_set_light(&hg->meta_out, light_no, intensity);
}

static bool
hid_generic_handle_event(struct hid_fd *hid_fd, OVERLAPPED *ovl, size_t nbytes)
{
    struct hid_generic *hg = containerof(hid_fd, struct hid_generic, super);

    if (ovl == &hg->out_ovl) {
        /* OUT transmit ok, prepare and transmit another */

        hid_meta_out_get_next_report(&hg->meta_out, hg->out_buf);
        memset(&hg->out_ovl, 0, sizeof(hg->out_ovl));

        if (!WriteFile(
                hg->fd, hg->out_buf, hg->out_nbytes, NULL, &hg->out_ovl) &&
            GetLastError() != ERROR_IO_PENDING) {
            log_warning(
                "Write error %08x from \"%s\" device on dev node %s",
                (unsigned int) GetLastError(),
                hg->strings.str_prod,
                hg->dev_node);

            return false;
        }

    } else if (ovl == &hg->in_ovl) {
        /* Received an IN report, dispatch it */

        /* The dispatcher will swap buffers with us, i.e. it will take
           ownership of whatever hg->current_in points to, and replace it
           with a pointer to a fresh buffer we can use for subsequent I/O */

        if (!hid_meta_in_dispatch(&hg->meta_in, &hg->in_buf, nbytes)) {
            log_warning(
                "Failed to process input report from \"%s\" device"
                " on dev node %s",
                hg->strings.str_prod,
                hg->dev_node);

            return false;
        }

        if (hg->in_buf == NULL) {
            /* BEEP BOOP FIRMWARE PROGRAMMED BY SHITLORD DETECTED */
            /* (ok, a less obnoxious explanation: The initial report
               solicitation failed, so the report handler did not have a
               valid buffer until we gave it one. This means that we have
               to allocate one for it to work with manually). */

            hg->in_buf = xmalloc(hid_meta_in_get_buffer_size(&hg->meta_in));
        }

        /* Event dispatched successfully. Kick off the next async read. */

        memset(&hg->in_ovl, 0, sizeof(hg->in_ovl));

        if (!ReadFile(hg->fd, hg->in_buf, hg->in_nbytes, NULL, &hg->in_ovl) &&
            GetLastError() != ERROR_IO_PENDING) {
            log_warning(
                "Read error %08x from \"%s\" device on dev node %s",
                (unsigned int) GetLastError(),
                hg->strings.str_prod,
                hg->dev_node);

            return false;
        }
    }

    return true;
}

static void hid_generic_close(struct hid *hid)
{
    struct hid_generic *hg = containerof(hid, struct hid_generic, super);

    /* Must close the FD first. I/O might be in flight and NT could potentially
       complete an I/O while we're freeing our buffers and completely trash our
       process heap by doing so. */

    CloseHandle(hg->fd);

    free(hg->dev_node);
    free(hg->out_buf);
    free(hg->in_buf);
    hid_meta_out_fini(&hg->meta_out);
    hid_meta_in_fini(&hg->meta_in);
    hid_generic_strings_fini(&hg->strings);

    HidD_FreePreparsedData(hg->ppd);

    free(hid);
}
