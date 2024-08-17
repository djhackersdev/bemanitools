#define LOG_MODULE "iidxio-ezusb"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "api/core/log.h"
#include "api/core/thread.h"

#include "ezusb/ezusb.h"
#include "ezusb/ezusbsys2.h"

#include "ezusb-iidx/ezusb-iidx.h"
#include "ezusb-iidx/fpga.h"
#include "ezusb-iidx/seg16-cmd.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/io/iidx.h"

#include "util/fs.h"
#include "util/time.h"

static HANDLE iidx_io_ezusb_handle;

static struct ezusb_iidx_msg_interrupt_read_packet iidx_io_ezusb_read_packet;
static struct ezusb_iidx_msg_interrupt_write_packet iidx_io_ezusb_write_packet;
static bool iidxio_io_ezusb_16seg_rts;

bool bt_io_iidx_ep1_send(void);
bool bt_io_iidx_ep2_recv(void);

bool bt_io_iidx_init()
{
    struct ezusb_ident ident;

    iidxio_io_ezusb_16seg_rts = false;

    log_info(
        "!!! IMPORTANT: Ensure that you have flashed the correct firmware "
        "to your hardware and the FPGA BEFORE running this !!!");

    log_misc("Opening device path %s...", EZUSB_DEVICE_PATH);

    iidx_io_ezusb_handle = ezusb_open(EZUSB_DEVICE_PATH);

    if (iidx_io_ezusb_handle == INVALID_HANDLE_VALUE) {
        log_fatal("Opening ezusb device failed");
        return false;
    } else {
        if (!ezusb_get_ident(iidx_io_ezusb_handle, &ident)) {
            log_fatal("Getting ezusb ident failed");
            return false;
        } else {
            log_info(
                "Connected ezusb: vid 0x%X, pid 0x%X", ident.vid, ident.pid);
        }
    }

    // Random data returned by device, likely not properly initalized on device
    // side Triggers random inputs and lights Flush that by execute a few polls
    for (int i = 0; i < 10; i++) {
        bt_io_iidx_ep2_recv();
    }

    return true;
}

void bt_io_iidx_fini(void)
{
    // Pushing some final state before closing the IO to the actual outputs,
    // e.g. lights on/off can be a bit finicky. Do a few polls to
    // "enforce"/flush this final state
    for (uint8_t i = 0; i < 5; i++) {
        bt_io_iidx_ep1_send();
        bt_io_iidx_ep2_recv();

        Sleep(10);
    }

    ezusb_close(iidx_io_ezusb_handle);
    iidx_io_ezusb_handle = INVALID_HANDLE_VALUE;
}

/* Total number of light bits is 33. That's slightly annoying. So, we pack
   the neons bit into an unused start btns light. The entire 32-bit word is
   then sent to geninput for output light mapping. */

void bt_io_iidx_ep1_deck_lights_set(uint16_t deck_lights)
{
    iidx_io_ezusb_write_packet.deck_lights = deck_lights;
}

void bt_io_iidx_ep1_panel_lights_set(uint8_t panel_lights)
{
    iidx_io_ezusb_write_packet.panel_lights = panel_lights;
}

void bt_io_iidx_ep1_top_lamps_set(uint8_t top_lamps)
{
    iidx_io_ezusb_write_packet.top_lamps = top_lamps;
}

void bt_io_iidx_ep1_top_neons_set(bool top_neons)
{
    iidx_io_ezusb_write_packet.top_neons = top_neons ? 1 : 0;
}

bool bt_io_iidx_ep1_send(void)
{
    BULK_TRANSFER_CONTROL transfer;
    uint32_t outpkt;

    iidx_io_ezusb_write_packet.node = EZUSB_IIDX_MSG_NODE_16SEG;
    iidx_io_ezusb_write_packet.cmd = EZUSB_IIDX_16SEG_CMD_WRITE;
    iidx_io_ezusb_write_packet.cmd_detail[0] = 0;
    iidx_io_ezusb_write_packet.cmd_detail[1] = 1;

    transfer.pipeNum = EZUSB_IIDX_MSG_PIPE_INTERRUPT_OUT;

    if (!ezusb_iidx_ioctl(
            iidx_io_ezusb_handle,
            IOCTL_EZUSB_BULK_WRITE,
            &transfer,
            sizeof(transfer),
            &iidx_io_ezusb_write_packet,
            sizeof(iidx_io_ezusb_write_packet),
            &outpkt)) {
        log_fatal("Failed to write interrupt endpoint of ezusb");
        return false;
    } else {
        return true;
    }
}

bool bt_io_iidx_ep2_recv(void)
{
    if (!ezusb_iidx_interrupt_read(
            iidx_io_ezusb_handle, &iidx_io_ezusb_read_packet)) {
        log_fatal("Failed to read interrupt endpoints of ezusb");
        return false;
    }

    // Wait for board to be ready to receive data on bulk endpoint
    // On older Windows platforms, just writing the the 16seg bulk endpoint all
    // the time was fine, probably because the driver was overall slower than on
    // newer platforms, e.g. Windows 10. There, you crash the C02 firmware AND
    // the kernel mode driver if you don't wait for the bulk endpoint to become
    // ready before writing to it.
    if (iidx_io_ezusb_read_packet.status == EZUSB_IIDX_16SEG_CMD_STATUS_OK) {
        iidxio_io_ezusb_16seg_rts = true;
    } else {
        iidxio_io_ezusb_16seg_rts = false;
    }

    return true;
}

uint8_t bt_io_iidx_ep2_turntable_get(uint8_t player_no)
{
    switch (player_no) {
        case 0:
            return iidx_io_ezusb_read_packet.p1_turntable;
        case 1:
            return iidx_io_ezusb_read_packet.p2_turntable;
        default:
            return 0;
    }
}

uint8_t bt_io_iidx_ep2_slider_get(uint8_t slider_no)
{
    switch (slider_no) {
        case 0:
            return iidx_io_ezusb_read_packet.sliders[0] & 0xF;
        case 1:
            return (iidx_io_ezusb_read_packet.sliders[0] >> 4) & 0xF;
        case 2:
            return iidx_io_ezusb_read_packet.sliders[1] & 0xF;
        case 3:
            return (iidx_io_ezusb_read_packet.sliders[1] >> 4) & 0xF;
        case 4:
            return iidx_io_ezusb_read_packet.sliders[2] & 0xF;
        default:
            return 0;
    }
}

uint8_t bt_io_iidx_ep2_sys_get(void)
{
    return (((~iidx_io_ezusb_read_packet.inverted_pad) >> 28) & 0x03) |
        ((((~iidx_io_ezusb_read_packet.inverted_pad) >> 22) & 0x01) << 2);
}

uint8_t bt_io_iidx_ep2_panel_get(void)
{
    return ((~iidx_io_ezusb_read_packet.inverted_pad) >> 24) & 0x0F;
}

uint16_t bt_io_iidx_ep2_keys_get(void)
{
    return ((~iidx_io_ezusb_read_packet.inverted_pad) >> 8) & 0x3FFF;
}

bool bt_io_iidx_ep3_16seg_send(const char *text)
{
    struct ezusb_iidx_msg_bulk_packet pkg;

    if (iidxio_io_ezusb_16seg_rts) {
        pkg.node = EZUSB_IIDX_MSG_NODE_16SEG;
        pkg.page = 0;
        memset(pkg.payload, ' ', sizeof(pkg.payload));
        memcpy(pkg.payload, text, 9);

        iidxio_io_ezusb_16seg_rts = false;

        Sleep(1);

        if (!ezusb_iidx_bulk_write(iidx_io_ezusb_handle, &pkg)) {
            log_fatal("Writing 16seg bulk package failed");
            return false;
        }
    }

    return true;
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_iidx_api_get(bt_io_iidx_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_iidx_init;
    api->v1.fini = bt_io_iidx_fini;
    api->v1.ep1_deck_lights_set = bt_io_iidx_ep1_deck_lights_set;
    api->v1.ep1_panel_lights_set = bt_io_iidx_ep1_panel_lights_set;
    api->v1.ep1_top_lamps_set = bt_io_iidx_ep1_top_lamps_set;
    api->v1.ep1_top_neons_set = bt_io_iidx_ep1_top_neons_set;
    api->v1.ep1_send = bt_io_iidx_ep1_send;
    api->v1.ep2_recv = bt_io_iidx_ep2_recv;
    api->v1.ep2_turntable_get = bt_io_iidx_ep2_turntable_get;
    api->v1.ep2_slider_get = bt_io_iidx_ep2_slider_get;
    api->v1.ep2_sys_get = bt_io_iidx_ep2_sys_get;
    api->v1.ep2_panel_get = bt_io_iidx_ep2_panel_get;
    api->v1.ep2_keys_get = bt_io_iidx_ep2_keys_get;
    api->v1.ep3_16seg_send = bt_io_iidx_ep3_16seg_send;
}
