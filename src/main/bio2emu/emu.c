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
#include "hooklib/rs232.h"

#include "bio2emu/emu.h"
#include "bio2emu/setupapi.h"

#include "imports/avs.h"

#include "util/defs.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"
#include "util/array.h"

static struct array bio2_active_ports;

void bio2emu_init()
{
    array_init(&bio2_active_ports);
    bio2emu_setupapi_hook_init(&bio2_active_ports);
}

void bio2emu_port_init(struct bio2emu_port* bio2_emu)
{
    // BIO2 seems like ACIO with just 1 device
    ac_io_emu_init(&bio2_emu->acio, bio2_emu->wport);
    rs232_hook_add_fd(bio2_emu->acio.fd);

    *array_append(struct bio2emu_port*, &bio2_active_ports) = bio2_emu;
}

void bio2emu_port_fini(struct bio2emu_port* bio2_emu)
{
    ac_io_emu_fini(&bio2_emu->acio);
}

HRESULT bio2emu_port_dispatch_irp(struct irp *irp)
{
    const struct ac_io_message *msg;
    HRESULT hr;

    log_assert(irp != NULL);

    struct bio2emu_port* check = NULL;
    struct bio2emu_port* selected_emu = NULL;

    for (size_t i = 0 ; i < bio2_active_ports.nitems ; i++) {
        check = *array_item(struct bio2emu_port*, &bio2_active_ports, i);
        if (ac_io_emu_match_irp(&check->acio, irp)) {
            selected_emu = check;
        }
    }

    if (!selected_emu) {
        return irp_invoke_next(irp);
    }

    struct ac_io_emu* emu = &selected_emu->acio;

    for (;;) {
        hr = ac_io_emu_dispatch_irp(emu, irp);

        if (hr != S_OK) {
            return hr;
        }

        msg = ac_io_emu_request_peek(emu);

        switch (msg->addr) {
        case 0:
            ac_io_emu_cmd_assign_addrs(emu, msg, 1);
            break;

        case 1:
            selected_emu->dispatcher(selected_emu, msg);
            break;

        case AC_IO_BROADCAST:
            log_warning("Broadcast(?) message on BIO2 bus?");
            break;

        default:
            log_warning("BIO2 message on unhandled bus address: %d", msg->addr);
            break;
        }

        ac_io_emu_request_pop(emu);
    }
}
