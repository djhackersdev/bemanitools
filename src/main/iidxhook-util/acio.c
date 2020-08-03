// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <devioctl.h>
#include <ntdef.h>
#include <ntddser.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include "acioemu/addr.h"
#include "acioemu/emu.h"
#include "acioemu/icca.h"

#include "hook/iohook.h"
#include "hooklib/rs232.h"

#include "iidxhook-util/acio.h"

#include "imports/avs.h"

#include "util/defs.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

static struct ac_io_emu iidxhook_util_acio_emu;
static struct ac_io_emu_icca iidxhook_util_acio_emu_icca[2];

static bool iidxhook_util_icca_override_version;
static enum ac_io_emu_icca_version iidxhook_util_icca_override_version_value;


void iidxhook_util_acio_override_version(enum ac_io_emu_icca_version version)
{
    iidxhook_util_icca_override_version = true;
    iidxhook_util_icca_override_version_value = version;
}

void iidxhook_util_acio_init(bool legacy_mode)
{
    uint8_t i;

    if (legacy_mode) {
        ac_io_legacy_mode();
    }

    ac_io_emu_init(&iidxhook_util_acio_emu, L"COM1");

    for (i = 0; i < lengthof(iidxhook_util_acio_emu_icca); i++) {
        ac_io_emu_icca_init(
            &iidxhook_util_acio_emu_icca[i], &iidxhook_util_acio_emu, i);
        if (iidxhook_util_icca_override_version) {
            ac_io_emu_icca_set_version(
                &iidxhook_util_acio_emu_icca[i],
                iidxhook_util_icca_override_version_value
            );
        }
    }

    rs232_hook_add_fd(iidxhook_util_acio_emu.fd);
}

void iidxhook_util_acio_fini(void)
{
    ac_io_emu_fini(&iidxhook_util_acio_emu);
}

HRESULT
iidxhook_util_acio_dispatch_irp(struct irp *irp)
{
    const struct ac_io_message *msg;
    HRESULT hr;

    log_assert(irp != NULL);

    if (!ac_io_emu_match_irp(&iidxhook_util_acio_emu, irp)) {
        return irp_invoke_next(irp);
    }

    for (;;) {
        hr = ac_io_emu_dispatch_irp(&iidxhook_util_acio_emu, irp);

        if (hr != S_OK) {
            return hr;
        }

        msg = ac_io_emu_request_peek(&iidxhook_util_acio_emu);

        switch (msg->addr) {
            case 0:
                ac_io_emu_cmd_assign_addrs(&iidxhook_util_acio_emu, msg, 2);

                break;

            case 1:
                ac_io_emu_icca_dispatch_request(
                    &iidxhook_util_acio_emu_icca[0], msg);

                break;

            case 2:
                ac_io_emu_icca_dispatch_request(
                    &iidxhook_util_acio_emu_icca[1], msg);

                break;

            case AC_IO_BROADCAST:
                log_warning("Broadcast(?) message on IIDX ACIO bus?");

                break;

            default:
                log_warning(
                    "ACIO message on unhandled bus address: %d", msg->addr);

                break;
        }

        ac_io_emu_request_pop(&iidxhook_util_acio_emu);
    }
}
