#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "bemanitools/iidxio.h"

#include "ezusb/util.h"
#include "ezusb2/ezusb2.h"

#include "ezusb2-iidx/ezusb2-iidx.h"

#include "util/fs.h"
#include "util/time.h"

#define EZUSB2_FIND_TIMEOUT_MS 10000

#define log_misc(...) iidx_io_log_misc("iidxio-ezusb2", __VA_ARGS__)
#define log_info(...) iidx_io_log_info("iidxio-ezusb2", __VA_ARGS__)
#define log_warning(...) iidx_io_log_warning("iidxio-ezusb2", __VA_ARGS__)
#define log_fatal(...) iidx_io_log_fatal("iidxio-ezusb2", __VA_ARGS__)

static log_formatter_t iidx_io_log_misc;
static log_formatter_t iidx_io_log_info;
static log_formatter_t iidx_io_log_warning;
static log_formatter_t iidx_io_log_fatal;

static HANDLE iidx_io_ezusb2_handle;

static struct ezusb2_iidx_msg_interrupt_read_packet iidx_io_ezusb2_read_packet;
static struct ezusb2_iidx_msg_interrupt_write_packet iidx_io_ezusb2_write_packet;

void iidx_io_set_loggers(log_formatter_t misc, log_formatter_t info,
        log_formatter_t warning, log_formatter_t fatal)
{
    iidx_io_log_misc = misc;
    iidx_io_log_info = info;
    iidx_io_log_warning = warning;
    iidx_io_log_fatal = fatal;
}

bool iidx_io_init(thread_create_t thread_create, thread_join_t thread_join,
        thread_destroy_t thread_destroy)
{
    struct ezusb_ident ident;
    char* device_path;
    uint64_t time_start;

    log_info("!!! IMPORTANT: Ensure that you have flashed the correct firmware "
        "to your hardware BEFORE running this !!!");

    log_misc("Finding connected ezusb2...");

    time_start = time_get_counter();

    do {
        device_path = ezusb2_find(&EZUSB2_GUID);

        if (device_path) {
            break;
        }

        log_misc("Failed to find connected ezusb2 device, retry...");
        Sleep(1000);
    } while (time_get_elapsed_ms(time_get_counter() - time_start) < 
            EZUSB2_FIND_TIMEOUT_MS);

    if (!device_path) {
        log_fatal("Could not find a connected ezusb2 device");
        return false;
    }

    log_misc("Found ezusb2 device at '%s', opening...", device_path);

    iidx_io_ezusb2_handle = ezusb2_open(device_path);

    free(device_path);

    if (iidx_io_ezusb2_handle == INVALID_HANDLE_VALUE) {
        log_fatal("Opening ezusb2 device failed");
        return false;
    } else {
        if (!ezusb2_get_ident(iidx_io_ezusb2_handle, &ident)) {
            log_fatal("Getting ezusb2 ident failed");
            return false;
        } else {
            log_info("Connected ezusb2: name %s, vid 0x%X, pid 0x%X", 
                ident.name, ident.vid, ident.pid);
            return true;
        }
    }
}

void iidx_io_fini(void)
{
    ezusb2_close(iidx_io_ezusb2_handle);
    iidx_io_ezusb2_handle = INVALID_HANDLE_VALUE;
}

/* Total number of light bits is 33. That's slightly annoying. So, we pack
   the neons bit into an unused start btns light. The entire 32-bit word is
   then sent to geninput for output light mapping. */

void iidx_io_ep1_set_deck_lights(uint16_t deck_lights)
{
    iidx_io_ezusb2_write_packet.deck_lights = deck_lights;
}

void iidx_io_ep1_set_panel_lights(uint8_t panel_lights)
{
    iidx_io_ezusb2_write_packet.panel_lights = panel_lights;
}

void iidx_io_ep1_set_top_lamps(uint8_t top_lamps)
{
    iidx_io_ezusb2_write_packet.top_lamps = top_lamps;
}

void iidx_io_ep1_set_top_neons(bool top_neons)
{
    iidx_io_ezusb2_write_packet.top_neons = top_neons ? 1 : 0;
}

bool iidx_io_ep1_send(void)
{
    if (!ezusb2_iidx_interrupt_write(iidx_io_ezusb2_handle, 
            &iidx_io_ezusb2_write_packet)) {
        log_fatal("Failed to write interrupt endpoint of ezusb");
        return false;
    } else {
        return true;
    }
}

bool iidx_io_ep2_recv(void)
{
    if (!ezusb2_iidx_interrupt_read(iidx_io_ezusb2_handle, 
            &iidx_io_ezusb2_read_packet)) {
        log_fatal("Failed to read interrupt endpoint of ezusb");
        return false;
    } else {
        return true;
    }
}

uint8_t iidx_io_ep2_get_turntable(uint8_t player_no)
{
    switch (player_no) {
        case 0:
            return iidx_io_ezusb2_read_packet.p1_turntable;
        case 1:
            return iidx_io_ezusb2_read_packet.p2_turntable;
        default:
            return 0;
    }
}

uint8_t iidx_io_ep2_get_slider(uint8_t slider_no)
{
    switch (slider_no) {
        case 0:
            return iidx_io_ezusb2_read_packet.sliders[0] & 0xF;
        case 1:
            return (iidx_io_ezusb2_read_packet.sliders[0] >> 4) & 0xF;
        case 2:
            return iidx_io_ezusb2_read_packet.sliders[1] & 0xF;
        case 3:
            return (iidx_io_ezusb2_read_packet.sliders[1] >> 4) & 0xF;
        case 4:
            return iidx_io_ezusb2_read_packet.sliders[2] & 0xF;
        default:
            return 0;
    }
}

uint8_t iidx_io_ep2_get_sys(void)
{
    return (((~iidx_io_ezusb2_read_packet.inverted_pad) >> 4) & 0x03) | 
        ((((~iidx_io_ezusb2_read_packet.inverted_pad) >> 30) & 1) << 2);
}

uint8_t iidx_io_ep2_get_panel(void)
{
    return ((~iidx_io_ezusb2_read_packet.inverted_pad) >> 0) & 0x0F;
}

uint16_t iidx_io_ep2_get_keys(void)
{
    return ((~iidx_io_ezusb2_read_packet.inverted_pad) >> 16) & 0x3FFF;
}

bool iidx_io_ep3_write_16seg(const char *text)
{
    /* 16seg writing to device done in ep2 */
    memcpy(iidx_io_ezusb2_write_packet.seg16, text, 9);
    return true;
}

