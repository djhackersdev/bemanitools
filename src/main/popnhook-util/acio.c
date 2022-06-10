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

#include "popnhook-util/acio.h"

#include "imports/avs.h"

#include "util/defs.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

static struct ac_io_emu popnhook_acio_emu;
static struct ac_io_emu_icca popnhook_acio_emu_icca;

static bool popnhook_icca_override_version;
static enum ac_io_emu_icca_version popnhook_icca_override_version_value;


void popnhook_acio_override_version(enum ac_io_emu_icca_version version)
{
    popnhook_icca_override_version = true;
    popnhook_icca_override_version_value = version;
}

void popnhook_acio_init(bool legacy_mode)
{
    if (legacy_mode) {
        ac_io_legacy_mode();
    }

    ac_io_emu_init(&popnhook_acio_emu, L"COM1");

    ac_io_emu_icca_init(
        &popnhook_acio_emu_icca, &popnhook_acio_emu, 0);
    if (popnhook_icca_override_version) {
        ac_io_emu_icca_set_version(
            &popnhook_acio_emu_icca,
            popnhook_icca_override_version_value
        );
    }

    rs232_hook_add_fd(popnhook_acio_emu.fd);
}

void popnhook_acio_fini(void)
{
    ac_io_emu_fini(&popnhook_acio_emu);
}

HRESULT
popnhook_acio_dispatch_irp(struct irp *irp)
{
    const struct ac_io_message *msg;
    HRESULT hr;

    log_assert(irp != NULL);

    if (!ac_io_emu_match_irp(&popnhook_acio_emu, irp)) {
        return iohook_invoke_next(irp);
    }

    for (;;) {
        hr = ac_io_emu_dispatch_irp(&popnhook_acio_emu, irp);

        if (hr != S_OK) {
            return hr;
        }

        msg = ac_io_emu_request_peek(&popnhook_acio_emu);

        switch (msg->addr) {
            case 0:
                ac_io_emu_cmd_assign_addrs(&popnhook_acio_emu, msg, 1);

                break;

            case 1:
                ac_io_emu_icca_dispatch_request(
                    &popnhook_acio_emu_icca, msg);

                break;

            case AC_IO_BROADCAST:
                log_warning("Broadcast(?) message on pop'n ACIO bus?");

                break;

            default:
                log_warning(
                    "ACIO message on unhandled bus address: %d", msg->addr);

                break;
        }

        ac_io_emu_request_pop(&popnhook_acio_emu);
    }
}
