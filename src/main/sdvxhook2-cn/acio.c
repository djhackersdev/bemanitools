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

#include "hook/iohook.h"
#include "hooklib/rs232.h"

#include "sdvxhook2-cn/acio.h"
#include "sdvxhook2-cn/kfca.h"

#include "imports/avs.h"

#include "util/defs.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

static struct ac_io_emu ac_io_emu;

void ac_io_port_init(void)
{
    ac_io_emu_init(&ac_io_emu, L"\\\\.\\COM2");

    kfca_init(&ac_io_emu);

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
        return irp_invoke_next(irp);
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
                kfca_dispatch_request(msg);

                break;

            case AC_IO_BROADCAST:
                log_warning("Broadcast? (0x70) message on ACIO bus");

                break;

            default:
                log_warning(
                    "ACIO message on unhandled bus address: %d", msg->addr);

                break;
        }

        ac_io_emu_request_pop(&ac_io_emu);
    }
}
