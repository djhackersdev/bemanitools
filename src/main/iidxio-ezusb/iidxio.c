// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "bemanitools/iidxio.h"

#include "ezusb/ezusb.h"
#include "ezusb/ezusbsys2.h"

#include "ezusb-iidx/ezusb-iidx.h"
#include "ezusb-iidx/fpga.h"
#include "ezusb-iidx/seg16-cmd.h"

#include "util/fs.h"
#include "util/time.h"

#define log_misc(...) iidx_io_log_misc("iidxio-ezusb", __VA_ARGS__)
#define log_info(...) iidx_io_log_info("iidxio-ezusb", __VA_ARGS__)
#define log_warning(...) iidx_io_log_warning("iidxio-ezusb", __VA_ARGS__)
#define log_fatal(...) iidx_io_log_fatal("iidxio-ezusb", __VA_ARGS__)

static log_formatter_t iidx_io_log_misc;
static log_formatter_t iidx_io_log_info;
static log_formatter_t iidx_io_log_warning;
static log_formatter_t iidx_io_log_fatal;

static HANDLE iidx_io_ezusb_handle;

static struct ezusb_iidx_msg_interrupt_read_packet iidx_io_ezusb_read_packet;
static struct ezusb_iidx_msg_interrupt_write_packet iidx_io_ezusb_write_packet;
static bool iidxio_io_ezusb_16seg_rts;

void iidx_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    iidx_io_log_misc = misc;
    iidx_io_log_info = info;
    iidx_io_log_warning = warning;
    iidx_io_log_fatal = fatal;
}

bool iidx_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
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

    // Random data returned by device, likely not properly initalized on device side
    // Triggers random inputs and lights
    // Flush that by execute a few polls
    for (int i = 0; i < 10; i++) {
        iidx_io_ep2_recv();
    }

    return true;
}

void iidx_io_fini(void)
{
    ezusb_close(iidx_io_ezusb_handle);
    iidx_io_ezusb_handle = INVALID_HANDLE_VALUE;
}

/* Total number of light bits is 33. That's slightly annoying. So, we pack
   the neons bit into an unused start btns light. The entire 32-bit word is
   then sent to geninput for output light mapping. */

void iidx_io_ep1_set_deck_lights(uint16_t deck_lights)
{
    iidx_io_ezusb_write_packet.deck_lights = deck_lights;
}

void iidx_io_ep1_set_panel_lights(uint8_t panel_lights)
{
    iidx_io_ezusb_write_packet.panel_lights = panel_lights;
}

void iidx_io_ep1_set_top_lamps(uint8_t top_lamps)
{
    iidx_io_ezusb_write_packet.top_lamps = top_lamps;
}

void iidx_io_ep1_set_top_neons(bool top_neons)
{
    iidx_io_ezusb_write_packet.top_neons = top_neons ? 1 : 0;
}

bool iidx_io_ep1_send(void)
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

bool iidx_io_ep2_recv(void)
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

uint8_t iidx_io_ep2_get_turntable(uint8_t player_no)
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

uint8_t iidx_io_ep2_get_slider(uint8_t slider_no)
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

uint8_t iidx_io_ep2_get_sys(void)
{
    return (((~iidx_io_ezusb_read_packet.inverted_pad) >> 28) & 0x03) |
        ((((~iidx_io_ezusb_read_packet.inverted_pad) >> 22) & 0x01) << 2);
}

uint8_t iidx_io_ep2_get_panel(void)
{
    return ((~iidx_io_ezusb_read_packet.inverted_pad) >> 24) & 0x0F;
}

uint16_t iidx_io_ep2_get_keys(void)
{
    return ((~iidx_io_ezusb_read_packet.inverted_pad) >> 8) & 0x3FFF;
}

bool iidx_io_ep3_write_16seg(const char *text)
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
