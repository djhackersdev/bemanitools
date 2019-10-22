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
#include "acioemu/hdxs.h"
#include "acioemu/icca.h"

#include "ddrhook/_com4.h"

#include "hook/iohook.h"

#include "p3ioemu/uart.h"

#include "util/defs.h"
#include "util/iobuf.h"
#include "util/log.h"

static struct ac_io_emu com4_ac_io_emu;
static struct ac_io_emu_hdxs com4_hdxs;
static struct ac_io_emu_icca com4_icca[2];

void com4_init(void)
{
    uint8_t i;

    /* This isn't a real COM port, we configure the core P3IO emulator code to
       generate IRPs addressed to COM4 and then we intercept them. */

    p3io_uart_set_path(0, L"COM4");
    ac_io_emu_init(&com4_ac_io_emu, L"COM4");
    ac_io_emu_hdxs_init(&com4_hdxs, &com4_ac_io_emu);

    for (i = 0; i < lengthof(com4_icca); i++) {
        ac_io_emu_icca_init(&com4_icca[i], &com4_ac_io_emu, i);
    }
}

void com4_fini(void)
{
    ac_io_emu_fini(&com4_ac_io_emu);
}

HRESULT
com4_dispatch_irp(struct irp *irp)
{
    const struct ac_io_message *msg;
    HRESULT hr;

    log_assert(irp != NULL);

    if (!ac_io_emu_match_irp(&com4_ac_io_emu, irp)) {
        return irp_invoke_next(irp);
    }

    for (;;) {
        hr = ac_io_emu_dispatch_irp(&com4_ac_io_emu, irp);

        if (hr != S_OK) {
            return hr;
        }

        msg = ac_io_emu_request_peek(&com4_ac_io_emu);

        switch (msg->addr) {
            case 0:
                ac_io_emu_cmd_assign_addrs(&com4_ac_io_emu, msg, 3);

                break;

            case 1:
                ac_io_emu_icca_dispatch_request(&com4_icca[0], msg);

                break;

            case 2:
                ac_io_emu_icca_dispatch_request(&com4_icca[1], msg);

                break;

            case 3:
                ac_io_emu_hdxs_dispatch_request(&com4_hdxs, msg);

                break;

            case AC_IO_BROADCAST:
                log_warning("Broadcast(?) message on p3io ACIO bus?");

                break;

            default:
                log_warning(
                    "p3io ACIO message on unhandled bus address: %d",
                    msg->addr);

                break;
        }

        ac_io_emu_request_pop(&com4_ac_io_emu);
    }
}
