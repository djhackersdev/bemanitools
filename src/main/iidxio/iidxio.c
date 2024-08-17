#define LOG_MODULE "iidxio"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>

#include "api/core/log.h"
#include "api/core/thread.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface-io/vefx.h"
#include "iface/input.h"

#include "main/module/input-ext.h"
#include "main/module/input.h"
#include "main/module/io-ext.h"
#include "main/module/io.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/input.h"
#include "sdk/module/io/iidx.h"

#define MSEC_PER_NOTCH 8

enum iidx_io_pad_bit {
    /* Synthetic inputs stuffed into unused bits in the pad word.

       These are not real inputs on the real IO board. Instead, these are
       convenience inputs returned from geninput, provided for the benefit of
       users who do not have analog turntables. */

    IIDX_IO_P1_TT_UP = 0x00,
    IIDX_IO_P1_TT_DOWN = 0x01,
    IIDX_IO_P1_TT_STAB = 0x02,

    IIDX_IO_P2_TT_UP = 0x03,
    IIDX_IO_P2_TT_DOWN = 0x04,
    IIDX_IO_P2_TT_STAB = 0x05,

    /* mapper_update() bit mappings (0x08 - 0x20) */

    IIDX_IO_P1_1 = 0x08,
    IIDX_IO_P1_2 = 0x09,
    IIDX_IO_P1_3 = 0x0A,
    IIDX_IO_P1_4 = 0x0B,
    IIDX_IO_P1_5 = 0x0C,
    IIDX_IO_P1_6 = 0x0D,
    IIDX_IO_P1_7 = 0x0E,

    IIDX_IO_P2_1 = 0x0F,
    IIDX_IO_P2_2 = 0x10,
    IIDX_IO_P2_3 = 0x11,
    IIDX_IO_P2_4 = 0x12,
    IIDX_IO_P2_5 = 0x13,
    IIDX_IO_P2_6 = 0x14,
    IIDX_IO_P2_7 = 0x15,

    IIDX_IO_P1_START = 0x18,
    IIDX_IO_P2_START = 0x19,
    IIDX_IO_VEFX = 0x1A,
    IIDX_IO_EFFECT = 0x1B,
    IIDX_IO_TEST = 0x1C,
    IIDX_IO_SERVICE = 0x1D,
};

struct iidx_io_tt_inputs {
    uint8_t tt_up;
    uint8_t tt_down;
    uint8_t tt_stab;
    uint8_t start;
};

struct iidx_io_tt {
    uint32_t last_notch;
    uint8_t pos;
    uint8_t analog_pos;
    int8_t stab_dir;
    int8_t last_dir;
};

static void iidx_io_tt_update(uint32_t now, uint64_t pad, int i);

static const struct iidx_io_tt_inputs iidx_io_tt_inputs[2] = {
    {IIDX_IO_P1_TT_UP,
     IIDX_IO_P1_TT_DOWN,
     IIDX_IO_P1_TT_STAB,
     IIDX_IO_P1_START},
    {IIDX_IO_P2_TT_UP,
     IIDX_IO_P2_TT_DOWN,
     IIDX_IO_P2_TT_STAB,
     IIDX_IO_P2_START},
};

static module_input_t *_iidx_io_module_input;
static module_io_t *_iidx_io_module_vefx;

static struct iidx_io_tt iidx_io_tt[2];
static uint8_t iidx_io_sys;
static uint8_t iidx_io_panel;
static uint16_t iidx_io_keys;

static void _bt_io_iidx_module_input_init(module_input_t **module)
{
    bt_input_api_t api;

    module_input_ext_load_and_init("geninput.dll", module);
    module_input_api_get(*module, &api);
    bt_input_api_set(&api);
}

static void _bt_io_iidx_module_vefx_init(module_io_t **module)
{
    bt_io_vefx_api_t api;

    module_io_ext_load_and_init(
        "vefxio.dll", "bt_module_io_vefx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_vefx_api_set(&api);
}

bool bt_io_iidx_init()
{
    bool result;

    _bt_io_iidx_module_input_init(&_iidx_io_module_input);
    _bt_io_iidx_module_vefx_init(&_iidx_io_module_vefx);

    result = bt_io_vefx_init();

    if (!result) {
        log_warning("Initializing vefx failed");
        return false;
    }

    timeBeginPeriod(1);

    result = bt_input_init();

    if (!result) {
        log_warning("Initializing input failed");
        return false;
    }

    iidx_io_tt[0].stab_dir = +1;
    iidx_io_tt[1].stab_dir = +1;

    return bt_input_mapper_config_load("iidx");
}

void bt_io_iidx_fini(void)
{
    bt_input_fini();
    bt_input_api_clear();
    module_input_free(&_iidx_io_module_input);

    bt_io_vefx_fini();
    bt_io_vefx_api_clear();
    module_io_free(&_iidx_io_module_vefx);

    timeEndPeriod(1);
}

void bt_io_iidx_ep1_deck_lights_set(uint16_t deck_lights)
{
    uint8_t i;

    for (i = 0x00; i < 0x0E; i++) {
        bt_input_mapper_light_write(i, deck_lights & (1 << i) ? 255 : 0);
    }
}

void bt_io_iidx_ep1_panel_lights_set(uint8_t panel_lights)
{
    uint8_t i;

    for (i = 0x00; i < 0x04; i++) {
        bt_input_mapper_light_write(
            0x18 + i, panel_lights & (1 << i) ? 255 : 0);
    }
}

void bt_io_iidx_ep1_top_lamps_set(uint8_t top_lamps)
{
    uint8_t i;

    for (i = 0x00; i < 0x08; i++) {
        bt_input_mapper_light_write(0x10 + i, top_lamps & (1 << i) ? 255 : 0);
    }
}

void bt_io_iidx_ep1_top_neons_set(bool top_neons)
{
    bt_input_mapper_light_write(0x1F, top_neons ? 255 : 0);
}

bool bt_io_iidx_ep1_send(void)
{
    return true;
}

bool bt_io_iidx_ep2_recv(void)
{
    uint32_t now;
    uint64_t pad;
    int i;

    /* Update all of our input state here. */

    now = timeGetTime();
    pad = (uint64_t) bt_input_mapper_update();
    bt_io_vefx_recv(&pad);

    for (i = 0; i < 2; i++) {
        iidx_io_tt_update(now, pad, i);
    }

    /* Mask out the stuff provided by geninput and store the pad state for
       later retrieval via iidx_io_ep2_get_pad() */

    iidx_io_sys = (pad >> IIDX_IO_TEST) & 0x03;
    iidx_io_panel = (pad >> IIDX_IO_P1_START) & 0x0F;
    iidx_io_keys = (pad >> IIDX_IO_P1_1) & 0x3FFF;

    return true;
}

static void iidx_io_tt_update(uint32_t now, uint64_t pad, int i)
{
    uint32_t delta_t;
    uint8_t notches;
    int8_t tt_dir;
    int8_t brake;

    /* Determine current turntable direction */

    if (pad & (1 << iidx_io_tt_inputs[i].tt_up)) {
        tt_dir = -1;
    } else if (pad & (1 << iidx_io_tt_inputs[i].tt_down)) {
        tt_dir = +1;
    } else if (pad & (1 << iidx_io_tt_inputs[i].tt_stab)) {
        tt_dir = iidx_io_tt[i].stab_dir;

        if (iidx_io_tt[i].last_dir == 0) {
            iidx_io_tt[i].stab_dir = -iidx_io_tt[i].stab_dir;
        }
    } else {
        tt_dir = 0;
    }

    /* Apply brakes if a start button is held */

    if (pad & (1 << iidx_io_tt_inputs[i].start)) {
        brake = 4;
    } else {
        brake = 1;
    }

    /* Update turntable based on current direction */

    if (tt_dir != iidx_io_tt[i].last_dir) {
        /* Just started (or stopped). Give the TT a big push to make sure
           it begins to register straight from the first frame */

        iidx_io_tt[i].pos += 4 * tt_dir;
        iidx_io_tt[i].last_notch = now;
    } else if (tt_dir != 0) {
        /* Roll TT forward by an appropriate number of notches, given the
           elapsed time. Roll `last_notch' forward by an appropriate number
           of msec to ensure partial notches get counted properly */

        delta_t = now - iidx_io_tt[i].last_notch;
        notches = delta_t / MSEC_PER_NOTCH / brake;

        iidx_io_tt[i].pos += tt_dir * notches;
        iidx_io_tt[i].last_notch += notches * MSEC_PER_NOTCH;
    }

    iidx_io_tt[i].last_dir = tt_dir;

    /* Snapshot analog spinner state as well. */

    iidx_io_tt[i].analog_pos = bt_input_mapper_analog_read(i);
}

uint8_t bt_io_iidx_ep2_turntable_get(uint8_t player_no)
{
    if (player_no > 1) {
        return 0;
    }

    return iidx_io_tt[player_no].pos + iidx_io_tt[player_no].analog_pos;
}

uint8_t bt_io_iidx_ep2_slider_get(uint8_t slider_no)
{
    return bt_io_vefx_slider_get(slider_no);
}

uint8_t bt_io_iidx_ep2_sys_get(void)
{
    return iidx_io_sys;
}

uint8_t bt_io_iidx_ep2_panel_get(void)
{
    return iidx_io_panel;
}

uint16_t bt_io_iidx_ep2_keys_get(void)
{
    return iidx_io_keys;
}

bool bt_io_iidx_ep3_16seg_send(const char *text)
{
    return bt_io_vefx_16seg_send(text);
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