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

#include "sdvxhook2/acio.h"

#include "imports/avs.h"

#include "util/defs.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

static struct ac_io_emu ac_io_emu;
static struct ac_io_emu_icca ac_io_emu_icca;

void ac_io_port_init(bool use_new_reader)
{
    if (use_new_reader) {
        ac_io_emu_init(&ac_io_emu, L"COM1");
        ac_io_emu_icca_init(&ac_io_emu_icca, &ac_io_emu, 0);
        ac_io_emu_icca_set_version(&ac_io_emu_icca, v170);
    } else {
        ac_io_emu_init(&ac_io_emu, L"\\\\.\\COM2");
        ac_io_emu_icca_init(&ac_io_emu_icca, &ac_io_emu, 0);
    }

    rs232_hook_add_fd(ac_io_emu.fd);
}

void ac_io_port_fini(void)
{
    ac_io_emu_fini(&ac_io_emu);
}

HRESULT ac_io_port_dispatch_irp(struct irp *irp)
{
    const struct ac_io_message *msg;
    HRESULT hr;

    log_assert(irp != NULL);

    if (!ac_io_emu_match_irp(&ac_io_emu, irp)) {
        return iohook_invoke_next(irp);
    }

    for (;;) {
        hr = ac_io_emu_dispatch_irp(&ac_io_emu, irp);

        if (hr != S_OK) {
            return hr;
        }

        msg = ac_io_emu_request_peek(&ac_io_emu);

        switch (msg->addr) {
            case 0:
                ac_io_emu_cmd_assign_addrs(&ac_io_emu, msg, 1);
                log_warning("ac_io_emu_cmd_assign_addrs");

                break;

            case 1:
                ac_io_emu_icca_dispatch_request(&ac_io_emu_icca, msg);

                break;

            case AC_IO_BROADCAST:
                log_warning("Broadcast? (0x70) message on BIO2 bus");

                break;

            default:
                log_warning(
                    "ACIO message on unhandled bus address: %d", msg->addr);

                break;
        }

        ac_io_emu_request_pop(&ac_io_emu);
    }
}
