#define LOG_MODULE "ddrio-p3io"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "bemanitools/ddrio.h"

#include "cconfig/cconfig-main.h"

#include "core/log.h"
#include "core/thread.h"

#include "extiodrv/device.h"
#include "extiodrv/extio.h"

#include "p3iodrv/ddr.h"
#include "p3iodrv/device.h"

#include "config.h"

static struct ddrio_p3io_config _ddr_io_p3io_config;
static HANDLE _ddr_io_p3io_handle;
static HANDLE _ddr_io_extio_handle;

static HRESULT _ddr_io_p3io_scan_and_open(HANDLE *handle)
{
    HRESULT hr;
    char path[MAX_PATH];

    log_info("Scanning for p3io...");

    hr = p3iodrv_device_scan(path);

    if (FAILED(hr)) {
        log_warning("Cannot find a connected p3io: %lX", hr);
        return hr;
    }

    log_info("Opening p3io: %s", path);

    hr = p3iodrv_device_open(path, handle);

    if (FAILED(hr)) {
        log_warning("Opening p3io failed: %lX", hr);
        return hr;
    }

    return hr;
}

static HRESULT _ddr_io_p3io_print_version(HANDLE handle)
{
    HRESULT hr;
    char version[P3IODRV_VERSION_MAX_LEN];

    memset(version, 0, P3IODRV_VERSION_MAX_LEN);

    hr = p3iodrv_device_read_version(handle, version);

    if (FAILED(hr)) {
        log_warning("Getting p3io version failed: %lX", hr);
        return hr;
    }

    log_info("P3IO version: %s", version);

    return hr;
}

static HRESULT _ddr_io_ddr_p3io_print_version(HANDLE handle)
{
    HRESULT hr;
    char str[4];
    uint32_t major;
    uint32_t minor;
    uint32_t patch;

    hr = p3iodrv_ddr_get_version(handle, str, &major, &minor, &patch);

    if (FAILED(hr)) {
        log_warning("Getting version failed: %lX", hr);
        return hr;
    }

    log_info("DDR version: %s %d.%d.%d", str, major, minor, patch);

    return hr;
}

static HRESULT _ddr_io_ddr_p3io_print_cabinet_type(HANDLE handle)
{
    HRESULT hr;
    enum p3io_cab_type type;

    hr = p3iodrv_ddr_get_cab_type(handle, &type);

    if (FAILED(hr)) {
        log_warning("Getting cab type failed: %lX", hr);
        return hr;
    }

    log_info("Cabinet type: %d", type);

    return hr;
}

static HRESULT _ddr_io_ddr_p3io_print_dip_switches(HANDLE handle)
{
    HRESULT hr;
    uint8_t dip_sw;

    hr = p3iodrv_ddr_get_dipsw(handle, &dip_sw);

    if (FAILED(hr)) {
        log_warning("Getting dip switches failed");
        return hr;
    }

    log_info("Dip switches: 0x%X", dip_sw);

    return hr;
}

static HRESULT _ddr_io_ddr_p3io_print_video_freq(HANDLE handle)
{
    HRESULT hr;
    enum p3io_video_freq video_freq;

    hr = p3iodrv_ddr_get_video_freq(handle, &video_freq);

    if (FAILED(hr)) {
        log_warning("Getting video freq failed");
        return hr;
    }

    log_info("Video freq: 0x%X", video_freq);

    return hr;
}

static HRESULT _ddr_io_ddr_p3io_print_debug_info(HANDLE handle)
{
    return _ddr_io_p3io_print_version(handle) &&
        _ddr_io_ddr_p3io_print_version(handle) &&
        _ddr_io_ddr_p3io_print_cabinet_type(handle) &&
        _ddr_io_ddr_p3io_print_dip_switches(handle) &&
        _ddr_io_ddr_p3io_print_video_freq(handle);
}

static HRESULT _ddr_io_p3io_init(HANDLE *handle)
{
    HRESULT hr;

    log_info("Initializing DDR P3IO...");

    hr = _ddr_io_p3io_scan_and_open(handle);

    if (FAILED(hr)) {
        return hr;
    }

    hr = p3iodrv_ddr_init(*handle);

    if (FAILED(hr)) {
        return hr;
    }

    hr = _ddr_io_ddr_p3io_print_debug_info(*handle);

    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}

static HRESULT _ddr_io_p3io_close(HANDLE *handle)
{
    HRESULT hr;

    log_info("Closing p3io...");

    hr = p3iodrv_device_close(handle);

    if (FAILED(hr)) {
        log_warning("Closing p3io failed: %lX", hr);
        return hr;
    }

    return hr;
}

static HRESULT _ddr_io_extio_init(HANDLE *handle)
{
    log_info("Initializing EXTIO...");

    return extiodrv_device_open(_ddr_io_p3io_config.extio_port, handle);
}

static HRESULT _ddr_io_extio_close(HANDLE *handle)
{
    log_info("Closing EXTIO...");

    return extiodrv_device_close(handle);
}

static HRESULT _ddr_io_flush_and_reset(HANDLE p3io_handle, HANDLE extio_handle)
{
    HRESULT hr;
    struct p3io_ddr_jamma jamma;
    struct p3io_ddr_output output;
    struct extiodrv_extio_pad_lights pad_lights[2];
    bool neons;

    log_assert(p3io_handle != INVALID_HANDLE_VALUE);
    log_assert(extio_handle != INVALID_HANDLE_VALUE);

    log_info("Flushing inputs and outputs");

    memset(&output, 0, sizeof(output));
    memset(&pad_lights, 0, sizeof(pad_lights));
    neons = false;

    hr = p3iodrv_ddr_get_jamma(p3io_handle, &jamma);

    if (FAILED(hr)) {
        log_warning("Reading jamma failed");
        return hr;
    }

    hr = p3iodrv_ddr_set_outputs(p3io_handle, &output);

    if (FAILED(hr)) {
        log_warning("Settings outputs failed");
        return hr;
    }

    hr = extiodrv_extio_transfer(
        extio_handle, EXTIODRV_EXTIO_SENSOR_READ_MODE_ALL, pad_lights, neons);

    if (FAILED(hr)) {
        log_warning("EXTIO transfer failed");
        return hr;
    }

    return hr;
}

void ddr_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    core_log_impl_set(misc, info, warning, fatal);
}

static void _ddr_io_config_init(struct ddrio_p3io_config *config_ddrio_p3io)
{
    struct cconfig *config;

    config = cconfig_init();

    ddrio_p3io_config_init(config);

    if (!cconfig_main_config_init(
            config,
            "--ddrio-p3io-config",
            "ddrio-p3io.conf",
            "--help",
            "-h",
            "ddrio-p3io",
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    ddrio_p3io_config_get(config_ddrio_p3io, config);

    cconfig_finit(config);
}

bool ddr_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    HRESULT hr;

    _ddr_io_config_init(&_ddr_io_p3io_config);

    hr = _ddr_io_p3io_init(&_ddr_io_p3io_handle);

    if (FAILED(hr)) {
        return false;
    }

    hr = _ddr_io_extio_init(&_ddr_io_extio_handle);

    if (FAILED(hr)) {
        return false;
    }

    hr = _ddr_io_flush_and_reset(_ddr_io_p3io_handle, _ddr_io_extio_handle);

    if (FAILED(hr)) {
        return false;
    }

    return true;
}

uint32_t ddr_io_read_pad(void)
{
    HRESULT hr;
    struct p3io_ddr_jamma jamma;

    hr = p3iodrv_ddr_get_jamma(_ddr_io_p3io_handle, &jamma);

    if (FAILED(hr)) {
        log_warning("Reading jamma failed");
        return 0;
    }

    return _byteswap_ulong(*((uint32_t *) &jamma));
}

void ddr_io_set_lights_extio(uint32_t extio_lights)
{
    HRESULT hr;
    struct extiodrv_extio_pad_lights pad_lights[EXTIO_PAD_LIGHT_MAX_PLAYERS];
    bool neons;

    pad_lights[0].up = (extio_lights & (1 << LIGHT_P1_UP)) > 0;
    pad_lights[0].down = (extio_lights & (1 << LIGHT_P1_DOWN)) > 0;
    pad_lights[0].left = (extio_lights & (1 << LIGHT_P1_LEFT)) > 0;
    pad_lights[0].right = (extio_lights & (1 << LIGHT_P1_RIGHT)) > 0;

    pad_lights[1].up = (extio_lights & (1 << LIGHT_P2_UP)) > 0;
    pad_lights[1].down = (extio_lights & (1 << LIGHT_P2_DOWN)) > 0;
    pad_lights[1].left = (extio_lights & (1 << LIGHT_P2_LEFT)) > 0;
    pad_lights[1].right = (extio_lights & (1 << LIGHT_P2_RIGHT)) > 0;

    neons = (extio_lights & (1 << LIGHT_NEONS)) > 0;

    hr = extiodrv_extio_transfer(
        _ddr_io_extio_handle,
        EXTIODRV_EXTIO_SENSOR_READ_MODE_ALL,
        pad_lights,
        neons);

    if (FAILED(hr)) {
        log_warning("EXTIO transfer failed: %lX", hr);
    }
}

void ddr_io_set_lights_p3io(uint32_t p3io_lights)
{
    HRESULT hr;
    struct p3io_ddr_output output;

    output.cabinet.top_p1_lower =
        (p3io_lights & (1 << LIGHT_P1_LOWER_LAMP)) > 0 ? 1 : 0;
    output.cabinet.top_p1_upper =
        (p3io_lights & (1 << LIGHT_P1_UPPER_LAMP)) > 0 ? 1 : 0;
    output.cabinet.top_p2_lower =
        (p3io_lights & (1 << LIGHT_P2_LOWER_LAMP)) > 0 ? 1 : 0;
    output.cabinet.top_p2_upper =
        (p3io_lights & (1 << LIGHT_P2_UPPER_LAMP)) > 0 ? 1 : 0;

    output.cabinet.p1_menu_buttons =
        (p3io_lights & (1 << LIGHT_P1_MENU)) > 0 ? 1 : 0;
    output.cabinet.p2_menu_buttons =
        (p3io_lights & (1 << LIGHT_P2_MENU)) > 0 ? 1 : 0;

    hr = p3iodrv_ddr_set_outputs(_ddr_io_p3io_handle, &output);

    if (FAILED(hr)) {
        log_warning("Settings outputs failed: %lX", hr);
    }
}

void ddr_io_set_lights_hdxs_panel(uint32_t lights)
{
    // Unused

    (void) lights;
}

void ddr_io_set_lights_hdxs_rgb(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    // Unused

    (void) idx;
    (void) r;
    (void) g;
    (void) b;
}

void ddr_io_fini(void)
{
    HRESULT hr;

    hr = _ddr_io_flush_and_reset(_ddr_io_p3io_handle, _ddr_io_extio_handle);

    if (FAILED(hr)) {
        log_warning("Flusing IO failed: %lX", hr);
        return;
    }

    hr = _ddr_io_p3io_close(&_ddr_io_p3io_handle);

    if (FAILED(hr)) {
        log_warning("Closing P3IO failed: %lX", hr);
        // continue
    }

    hr = _ddr_io_extio_close(&_ddr_io_extio_handle);

    if (FAILED(hr)) {
        log_warning("Closing EXTIO failed: %lX", hr);
        // continue
    }
}
