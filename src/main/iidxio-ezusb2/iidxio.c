#define LOG_MODULE "iidxio-ezusb2"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "api/core/log.h"
#include "api/core/thread.h"

#include "ezusb/util.h"
#include "ezusb2/ezusb2.h"

#include "ezusb2-iidx/ezusb2-iidx.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/io/iidx.h"

#include "util/fs.h"
#include "util/time.h"

#define EZUSB2_FIND_TIMEOUT_MS 10000

static HANDLE iidx_io_ezusb2_handle;

static struct ezusb2_iidx_msg_interrupt_read_packet iidx_io_ezusb2_read_packet;
static struct ezusb2_iidx_msg_interrupt_write_packet
    iidx_io_ezusb2_write_packet;

bool bt_io_iidx_init()
{
    struct ezusb_ident ident;
    char *device_path;
    uint64_t time_start;

    log_info(
        "!!! IMPORTANT: Ensure that you have flashed the correct firmware "
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
            log_info(
                "Connected ezusb2: name %s, vid 0x%X, pid 0x%X",
                ident.name,
                ident.vid,
                ident.pid);
            return true;
        }
    }
}

void bt_io_iidx_fini(void)
{
    ezusb2_close(iidx_io_ezusb2_handle);
    iidx_io_ezusb2_handle = INVALID_HANDLE_VALUE;
}

void bt_io_iidx_ep1_deck_lights_set(uint16_t deck_lights)
{
    iidx_io_ezusb2_write_packet.deck_lights = deck_lights;
}

void bt_io_iidx_ep1_panel_lights_set(uint8_t panel_lights)
{
    iidx_io_ezusb2_write_packet.panel_lights = panel_lights;
}

void bt_io_iidx_ep1_top_lamps_set(uint8_t top_lamps)
{
    iidx_io_ezusb2_write_packet.top_lamps = top_lamps;
}

void bt_io_iidx_ep1_top_neons_set(bool top_neons)
{
    iidx_io_ezusb2_write_packet.top_neons = top_neons ? 1 : 0;
}

bool bt_io_iidx_ep1_send(void)
{
    if (!ezusb2_iidx_interrupt_write(
            iidx_io_ezusb2_handle, &iidx_io_ezusb2_write_packet)) {
        log_fatal("Failed to write interrupt endpoint of ezusb");
        return false;
    } else {
        return true;
    }
}

bool bt_io_iidx_ep2_recv(void)
{
    if (!ezusb2_iidx_interrupt_read(
            iidx_io_ezusb2_handle, &iidx_io_ezusb2_read_packet)) {
        log_fatal("Failed to read interrupt endpoint of ezusb");
        return false;
    } else {
        return true;
    }
}

uint8_t bt_io_iidx_ep2_turntable_get(uint8_t player_no)
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

uint8_t bt_io_iidx_ep2_slider_get(uint8_t slider_no)
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

uint8_t bt_io_iidx_ep2_sys_get(void)
{
    return (((~iidx_io_ezusb2_read_packet.inverted_pad) >> 4) & 0x03) |
        ((((~iidx_io_ezusb2_read_packet.inverted_pad) >> 30) & 1) << 2);
}

uint8_t bt_io_iidx_ep2_panel_get(void)
{
    return ((~iidx_io_ezusb2_read_packet.inverted_pad) >> 0) & 0x0F;
}

uint16_t bt_io_iidx_ep2_keys_get(void)
{
    return ((~iidx_io_ezusb2_read_packet.inverted_pad) >> 16) & 0x3FFF;
}

bool bt_io_iidx_ep3_16seg_send(const char *text)
{
    /* 16seg writing to device done in ep2 */
    memcpy(iidx_io_ezusb2_write_packet.seg16, text, 9);
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
