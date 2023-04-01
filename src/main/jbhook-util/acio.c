// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <ntdef.h>
#include <devioctl.h>
#include <ntddser.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include "acioemu/addr.h"
#include "acioemu/emu.h"
#include "acioemu/h44b.h"
#include "acioemu/icca.h"

#include "hook/iohook.h"

#include "jbhook-util/acio.h"

#include "imports/avs.h"

#include "util/defs.h"
#include "util/hex.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

static struct ac_io_emu ac_io_emu;
static struct ac_io_emu_icca ac_io_emu_icca;
static struct ac_io_emu_h44b ac_io_emu_h44b;

void jbhook_util_ac_io_port_init(const wchar_t *filename)
{
    ac_io_emu_init(&ac_io_emu, filename);
    ac_io_emu_icca_init(&ac_io_emu_icca, &ac_io_emu, 0);
    ac_io_emu_h44b_init(&ac_io_emu_h44b, &ac_io_emu, 1);
}

void jbhook_util_ac_io_port_fini(void)
{
    ac_io_emu_fini(&ac_io_emu);
}

void jbhook_util_ac_io_set_iccb(void)
{
    ac_io_emu_icca_set_version(&ac_io_emu_icca, v150);
    ac_io_emu_icca_set_product_code(&ac_io_emu_icca, AC_IO_EMU_PROD_CODE_ICCB);
}

HRESULT
jbhook_util_ac_io_port_dispatch_irp(struct irp *irp)
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
                ac_io_emu_cmd_assign_addrs(&ac_io_emu, msg, 2);

                break;

            case 1:
                ac_io_emu_icca_dispatch_request(&ac_io_emu_icca, msg);

                break;

            case 2:
                ac_io_emu_h44b_dispatch_request(&ac_io_emu_h44b, msg);

                break;

            case AC_IO_BROADCAST:
                log_warning("Broadcast(?) message on jubeat ACIO bus?");

                break;

            default:
                log_warning(
                    "ACIO message on unhandled bus address: %d", msg->addr);

                break;
        }

        ac_io_emu_request_pop(&ac_io_emu);
    }
}
