#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/ddrio.h"

#include "ddrhook/p3io.h"

#include "p3ioemu/emu.h"
#include "p3ioemu/uart.h"

#include "util/log.h"

extern bool _15khz;
extern bool standard_def;

static HRESULT p3io_ddr_read_jamma(void *ctx, uint32_t *state);
static HRESULT p3io_ddr_set_outputs(void *ctx, uint32_t outputs);
static HRESULT p3io_ddr_get_cab_type(void *ctx, enum p3io_cab_type *type);
static HRESULT p3io_ddr_get_video_freq(void *ctx, enum p3io_video_freq *freq);

static const struct p3io_ops p3io_ddr_ops = {
    .read_jamma = p3io_ddr_read_jamma,
    .set_outputs = p3io_ddr_set_outputs,
    .get_cab_type = p3io_ddr_get_cab_type,
    .get_video_freq = p3io_ddr_get_video_freq,
};

void p3io_ddr_init(void)
{
    /* COM4 isn't a real COM port, we configure the core P3IO emulator code to
       generate IRPs addressed to COM4 and then we possibly intercept them in
       _com4.c (or possibly don't, in which case the communications will be
       sent to the real COM4 on your system) */

    p3io_uart_set_path(0, L"COM4");
    p3io_emu_init(&p3io_ddr_ops, NULL);
}

void p3io_ddr_fini(void)
{
    p3io_emu_fini();
}

static HRESULT p3io_ddr_read_jamma(void *ctx, uint32_t *state)
{
    log_assert(state != NULL);

    *state = ddr_io_read_pad();

    return S_OK;
}

static HRESULT p3io_ddr_set_outputs(void *ctx, uint32_t outputs)
{
    ddr_io_set_lights_p3io(outputs);

    return S_OK;
}

static HRESULT p3io_ddr_get_cab_type(void *ctx, enum p3io_cab_type *type)
{
    if (standard_def) {
        *type = P3IO_CAB_TYPE_SD;
    } else {
        *type = P3IO_CAB_TYPE_HD;
    }

    return S_OK;
}

static HRESULT p3io_ddr_get_video_freq(void *ctx, enum p3io_video_freq *freq)
{
    if (_15khz) {
        *freq = P3IO_VIDEO_FREQ_15KHZ;
    } else {
        *freq = P3IO_VIDEO_FREQ_31KHZ;
    }

    return S_OK;
}

// TODO coinstock
// TODO round plug
