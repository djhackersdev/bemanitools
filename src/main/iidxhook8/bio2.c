#include <windows.h>

#include <ntdef.h>
#include <devioctl.h>
#include <ntddser.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include "acioemu/addr.h"
#include "acioemu/emu.h"

#include "hook/iohook.h"

#include "iidxhook8/bi2a.h"

#include "imports/avs.h"

#include "util/defs.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

static struct ac_io_emu bio2_emu;

void bio2_port_init(bool disable_poll_limiter)
{
    // yes I'm using acio
    // they use the same framing
    ac_io_emu_init(&bio2_emu, L"COM4");
    bio2_emu_bi2a_init(&bio2_emu, disable_poll_limiter);
}

void bio2_port_fini(void)
{
    ac_io_emu_fini(&bio2_emu);
}

HRESULT bio2_port_dispatch_irp(struct irp *irp)
{
    const struct ac_io_message *msg;
    HRESULT hr;

    log_assert(irp != NULL);

    if (!ac_io_emu_match_irp(&bio2_emu, irp)) {
        return irp_invoke_next(irp);
    }

    for (;;) {
        hr = ac_io_emu_dispatch_irp(&bio2_emu, irp);

        if (hr != S_OK) {
            return hr;
        }

        msg = ac_io_emu_request_peek(&bio2_emu);

        switch (msg->addr) {
        case 0:
            ac_io_emu_cmd_assign_addrs(&bio2_emu, msg, 1);
            break;

        case 1:
            bio2_emu_bi2a_dispatch_request(msg);
            break;

        case AC_IO_BROADCAST:
            log_warning("Broadcast(?) message on IIDX BIO2 bus?");
            break;

        default:
            log_warning("BIO2 message on unhandled bus address: %d", msg->addr);
            break;
        }

        ac_io_emu_request_pop(&bio2_emu);
    }
}
