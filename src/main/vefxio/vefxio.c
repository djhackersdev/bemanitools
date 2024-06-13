#define LOG_MODULE "vefxio"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>

#include "iface-core/log.h"
#include "iface-io/vefx.h"

#include "sdk/module/io/vefx.h"

#define MSEC_PER_NOTCH 128

enum iidx_slider_bits {
    VEFX_IO_S1_UP = 0x20,
    VEFX_IO_S1_DOWN = 0x21,

    VEFX_IO_S2_UP = 0x22,
    VEFX_IO_S2_DOWN = 0x23,

    VEFX_IO_S3_UP = 0x24,
    VEFX_IO_S3_DOWN = 0x25,

    VEFX_IO_S4_UP = 0x26,
    VEFX_IO_S4_DOWN = 0x27,

    VEFX_IO_S5_UP = 0x28,
    VEFX_IO_S5_DOWN = 0x29,
};

struct vefx_io_slider_inputs {
    uint8_t slider_up;
    uint8_t slider_down;
};

struct vefx_io_slider {
    uint32_t last_notch;
    int8_t pos;
    int8_t last_dir;
};

static const struct vefx_io_slider_inputs vefx_io_slider_inputs[5] = {
    {VEFX_IO_S1_UP, VEFX_IO_S1_DOWN},
    {VEFX_IO_S2_UP, VEFX_IO_S2_DOWN},
    {VEFX_IO_S3_UP, VEFX_IO_S3_DOWN},
    {VEFX_IO_S4_UP, VEFX_IO_S4_DOWN},
    {VEFX_IO_S5_UP, VEFX_IO_S5_DOWN},
};

static struct vefx_io_slider vefx_io_slider[5];

static void vefx_io_slider_update(uint64_t *ppad);

bool bt_io_vefx_init()
{
    /* geninput should have already been initted be now so we don't do it */
    vefx_io_slider[0].pos = 8;
    vefx_io_slider[1].pos = 8;
    vefx_io_slider[2].pos = 8;
    vefx_io_slider[3].pos = 15;
    vefx_io_slider[4].pos = 15;

    /* Initialize your own IO devices here. Log something and then return
       false if the initialization fails. */

    return true;
}

bool bt_io_vefx_recv(uint64_t *ppad)
{
    vefx_io_slider_update(ppad);

    return true;
}

void bt_io_vefx_fini(void)
{
    // noop
}

static void vefx_io_slider_update(uint64_t *ppad)
{
    uint32_t delta;
    int sno;
    int8_t sdir;
    uint32_t now;
    uint64_t pad;

    pad = *ppad;
    now = timeGetTime();

    for (sno = 0; sno < 5; sno++) {
        if (pad & (1ULL << vefx_io_slider_inputs[sno].slider_up)) {
            sdir = +1;
        } else if (pad & (1ULL << vefx_io_slider_inputs[sno].slider_down)) {
            sdir = -1;
        } else {
            sdir = 0;
        }

        /* Update slider based on current direction */

        if (sdir != vefx_io_slider[sno].last_dir) {
            /* Just started (or stopped). Give the slider a big push to make
            sure it begins to register straight from the first frame */

            vefx_io_slider[sno].pos += sdir;
            vefx_io_slider[sno].last_notch = now;
        } else if (sdir != 0) {
            /* Roll slider forward by an appropriate number of notches, given
            the elapsed time. Roll `last_notch' forward by an appropriate number
            of msec to ensure partial notches get counted properly */

            delta = now - vefx_io_slider[sno].last_notch;
            if (delta >= MSEC_PER_NOTCH) {
                vefx_io_slider[sno].pos += sdir;
                vefx_io_slider[sno].last_notch = now;
            }
        }
        vefx_io_slider[sno].last_dir = sdir;
        if (vefx_io_slider[sno].pos < 0) {
            vefx_io_slider[sno].pos = 0;
        }
        if (vefx_io_slider[sno].pos > 15) {
            vefx_io_slider[sno].pos = 15;
        }
    }
}

uint8_t bt_io_vefx_slider_get(uint8_t slider_no)
{
    if (slider_no > 4) {
        return 0;
    }

    return vefx_io_slider[slider_no].pos;
}

bool bt_io_vefx_16seg_send(const char *text)
{
    /* Insert code to write to your 16seg display here.
       Log something and return false if you encounter an IO error. */

    return true;
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_io_vefx_api_get(bt_io_vefx_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_vefx_init;
    api->v1.fini = bt_io_vefx_fini;
    api->v1.recv = bt_io_vefx_recv;
    api->v1.slider_get = bt_io_vefx_slider_get;
    api->v1._16seg_send = bt_io_vefx_16seg_send;
}