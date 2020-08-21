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

#include "acio/hdxs.h"

#include "acioemu/addr.h"
#include "acioemu/hdxs.h"
#include "acioemu/icca.h"

#include "bemanitools/ddrio.h"

#include "ddrhook/_com4.h"

#include "hook/iohook.h"

#include "p3ioemu/uart.h"

#include "util/defs.h"
#include "util/iobuf.h"
#include "util/log.h"

static struct ac_io_emu com4_ac_io_emu;
static struct ac_io_emu_hdxs com4_hdxs;
static struct ac_io_emu_icca com4_icca[2];

static uint32_t check_panel_light(
    const struct ac_io_hdxs_output *output,
    uint8_t panel_idx,
    uint8_t out_idx)
{
    if (output->lights[panel_idx].bit) {
        return 1 << out_idx;
    } else {
        return 0;
    }
}

static uint8_t upscale_light(uint8_t in_7bit) {
    if (in_7bit < 0x10) {
        return in_7bit * 2;
    } else {
        // so we can actually reach 0xFF
        return (in_7bit * 2) + 1;
    }
}

static void lights_dispatcher(
    struct ac_io_emu_hdxs *emu, const struct ac_io_message *req)
{
    const struct ac_io_hdxs_output *output = &req->cmd.hdxs_output;

    uint32_t lights = 0;
    lights |= check_panel_light(output, AC_IO_HDXS_OUT_P1_START, LIGHT_HD_P1_START);
    lights |= check_panel_light(output, AC_IO_HDXS_OUT_P1_UP_DOWN, LIGHT_HD_P1_UP_DOWN);
    lights |= check_panel_light(output, AC_IO_HDXS_OUT_P1_LEFT_RIGHT, LIGHT_HD_P1_LEFT_RIGHT);
    lights |= check_panel_light(output, AC_IO_HDXS_OUT_P2_START, LIGHT_HD_P2_START);
    lights |= check_panel_light(output, AC_IO_HDXS_OUT_P2_UP_DOWN, LIGHT_HD_P2_UP_DOWN);
    lights |= check_panel_light(output, AC_IO_HDXS_OUT_P2_LEFT_RIGHT, LIGHT_HD_P2_LEFT_RIGHT);

    ddr_io_set_lights_hdxs_panel(lights);

    for (uint8_t i = 0; i < 4; ++i) {
        size_t light_idx = i * 3;

        // these are 7 bit, upscale them to 8 bit
        uint8_t r = upscale_light(output->lights[light_idx + AC_IO_HDXS_RED].analog);
        uint8_t g = upscale_light(output->lights[light_idx + AC_IO_HDXS_GREEN].analog);
        uint8_t b = upscale_light(output->lights[light_idx + AC_IO_HDXS_BLUE].analog);

        ddr_io_set_lights_hdxs_rgb(i, r, g, b);
    }
}

void com4_init(void)
{
    uint8_t i;

    ac_io_emu_init(&com4_ac_io_emu, L"COM4");
    ac_io_emu_hdxs_init(&com4_hdxs, &com4_ac_io_emu, lights_dispatcher);

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
        return iohook_invoke_next(irp);
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
