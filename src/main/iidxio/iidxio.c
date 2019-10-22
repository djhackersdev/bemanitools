/* This is the source code for the IIDXIO.DLL that ships with Bemanitools 5.

   If you want to add on some minor functionality like a custom 16seg display
   or a customer slider board then see vefxio.

   If you want to make a completely custom IO board that handles all input and
   lighting then you'd be better off writing your own from scratch. Consult
   the "bemanitools" header files included by this source file for detailed
   information about the API you'll need to implement. */

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/iidxio.h"
#include "bemanitools/input.h"
#include "bemanitools/vefxio.h"

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

static struct iidx_io_tt iidx_io_tt[2];
static uint8_t iidx_io_sys;
static uint8_t iidx_io_panel;
static uint16_t iidx_io_keys;

/* Uncomment these if you need them. */

#if 0
static log_formatter_t iidx_io_log_misc;
static log_formatter_t iidx_io_log_info;
static log_formatter_t iidx_io_log_warning;
static log_formatter_t iidx_io_log_fatal;
#endif

void iidx_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    /* Pass logger functions on to geninput so that it has somewhere to write
       its own log output. */

    input_set_loggers(misc, info, warning, fatal);

    /* Pass logger functions on to vefx_io so that it has somewhere to write
       its own log output. */

    vefx_io_set_loggers(misc, info, warning, fatal);

    /* Uncomment this block if you have something you'd like to log.

       You should probably return false from the appropriate function instead
       of calling the fatal logger yourself though. */

#if 0
    iidx_io_log_misc = misc;
    iidx_io_log_info = info;
    iidx_io_log_warning = warning;
    iidx_io_log_fatal = fatal;
#endif
}

bool iidx_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    vefx_io_init(thread_create, thread_join, thread_destroy);
    timeBeginPeriod(1);

    input_init(thread_create, thread_join, thread_destroy);
    mapper_config_load("iidx");

    iidx_io_tt[0].stab_dir = +1;
    iidx_io_tt[1].stab_dir = +1;

    /* Initialize your own IO devices here. Log something and then return
       false if the initialization fails. */

    return true;
}

void iidx_io_fini(void)
{
    /* This function gets called as IIDX shuts down after an Alt-F4. Close your
       connections to your IO devices here. */

    input_fini();
    vefx_io_fini();
    timeEndPeriod(1);
}

/* Total number of light bits is 33. That's slightly annoying. So, we pack
   the neons bit into an unused start btns light. The entire 32-bit word is
   then sent to geninput for output light mapping. */

void iidx_io_ep1_set_deck_lights(uint16_t deck_lights)
{
    uint8_t i;

    for (i = 0x00; i < 0x0E; i++) {
        mapper_write_light(i, deck_lights & (1 << i) ? 255 : 0);
    }
}

void iidx_io_ep1_set_panel_lights(uint8_t panel_lights)
{
    uint8_t i;

    for (i = 0x00; i < 0x04; i++) {
        mapper_write_light(0x18 + i, panel_lights & (1 << i) ? 255 : 0);
    }
}

void iidx_io_ep1_set_top_lamps(uint8_t top_lamps)
{
    uint8_t i;

    for (i = 0x00; i < 0x08; i++) {
        mapper_write_light(0x10 + i, top_lamps & (1 << i) ? 255 : 0);
    }
}

void iidx_io_ep1_set_top_neons(bool top_neons)
{
    mapper_write_light(0x1F, top_neons ? 255 : 0);
}

bool iidx_io_ep1_send(void)
{
    /* The generic input stack currently initiates lighting sends and input
       reads simultaneously, though this might change later. Perform all of our
       I/O immediately before reading out the inputs so that the input state is
       as fresh as possible. */

    return true;
}

bool iidx_io_ep2_recv(void)
{
    uint32_t now;
    uint64_t pad;
    int i;

    /* Update all of our input state here. */

    now = timeGetTime();
    pad = (uint64_t) mapper_update();
    vefx_io_recv(&pad);

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

    iidx_io_tt[i].analog_pos = mapper_read_analog(i);
}

uint8_t iidx_io_ep2_get_turntable(uint8_t player_no)
{
    if (player_no > 1) {
        return 0;
    }

    return iidx_io_tt[player_no].pos + iidx_io_tt[player_no].analog_pos;
}

uint8_t iidx_io_ep2_get_slider(uint8_t slider_no)
{
    return vefx_io_get_slider(slider_no);
}

uint8_t iidx_io_ep2_get_sys(void)
{
    return iidx_io_sys;
}

uint8_t iidx_io_ep2_get_panel(void)
{
    return iidx_io_panel;
}

uint16_t iidx_io_ep2_get_keys(void)
{
    return iidx_io_keys;
}

bool iidx_io_ep3_write_16seg(const char *text)
{
    return vefx_io_write_16seg(text);
}
