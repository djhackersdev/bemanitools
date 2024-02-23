#define LOG_MODULE "p3io-ddr-tool"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"
#include "core/log.h"

#include "extiodrv/extio.h"

#include "p3io/ddr.h"
#include "p3iodrv/ddr.h"
#include "p3iodrv/device.h"

#include "mode-test.h"

enum mode {
    MODE_INVALID = 0,
    MODE_SCAN = 1,
    MODE_VERP3IO = 2,
    MODE_INIT = 3,
    MODE_VERDDR = 4,
    MODE_WATCHDOGON = 5,
    MODE_WATCHDOGOFF = 6,
    MODE_CABTYPE = 7,
    MODE_DIPSW = 8,
    MODE_VIDEOFREQ = 9,
    MODE_TEST = 10,
    MODE_LIGHTOFF = 11,
    MODE_POLL = 12,
    MODE_SENSORES = 13,
    MODE_TOTAL_COUNT = 14,
};

typedef bool (*mode_proc)(HANDLE handle);

static bool _mode_invalid(HANDLE handle);
static bool _mode_scan(HANDLE handle);
static bool _mode_verp3io(HANDLE handle);
static bool _mode_init(HANDLE handle);
static bool _mode_verddr(HANDLE handle);
static bool _mode_watchdogon(HANDLE handle);
static bool _mode_watchdogoff(HANDLE handle);
static bool _mode_cabtype(HANDLE handle);
static bool _mode_dipsw(HANDLE handle);
static bool _mode_videofreq(HANDLE handle);
static bool _mode_test(HANDLE handle);
static bool _mode_lightoff(HANDLE handle);
static bool _mode_poll(HANDLE handle);
static bool _mode_sensores(HANDLE handle);

static mode_proc _mode_procs[MODE_TOTAL_COUNT] = {
    _mode_invalid,
    _mode_scan,
    _mode_verp3io,
    _mode_init,
    _mode_verddr,
    _mode_watchdogon,
    _mode_watchdogoff,
    _mode_cabtype,
    _mode_dipsw,
    _mode_videofreq,
    _mode_test,
    _mode_lightoff,
    _mode_poll,
    _mode_sensores,
};

static enum core_log_bt_log_level _log_level = CORE_LOG_BT_LOG_LEVEL_FATAL;
static bool _extio_enabled = true;
static const char *_p3io_device_path = "";
static const char *_extio_com_port = "COM1";
static enum mode _mode = MODE_INVALID;

static bool _scan_and_open(HANDLE *handle)
{
    HRESULT hr;
    char path[MAX_PATH];

    log_info("Scanning for p3io...");

    hr = p3iodrv_device_scan(path);

    if (FAILED(hr)) {
        log_warning("Cannot find a connected p3io: %lX", hr);
        return false;
    }

    log_info("Opening p3io: %s", path);

    hr = p3iodrv_device_open(path, handle);

    if (FAILED(hr)) {
        log_warning("Opening p3io failed: %lX", hr);
        return false;
    }

    return true;
}

static bool _close(HANDLE handle)
{
    HRESULT hr;

    log_info("Closing p3io...");

    hr = p3iodrv_device_close(handle);

    if (FAILED(hr)) {
        log_warning("Closing p3io failed: %lX", hr);
        return false;
    }

    return true;
}

static bool _process_cmd_args(int argc, char **argv)
{
    char *mode;

    if (argc < 2) {
        fprintf(stderr, "DDR P3IO and EXTIO CLI testing tool\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage: [ARG ...] MODE\n");
        fprintf(stderr, "Arguments:\n");
        fprintf(
            stderr,
            "  -v        Enable additional logging, log level 'warning'\n");
        fprintf(
            stderr,
            "  -vv       Enable additional logging, log level 'info'\n");
        fprintf(
            stderr,
            "  -vvv      Enable additional logging, log level 'misc' (very "
            "verbose)\n");
        fprintf(
            stderr,
            "  -noextio  Run the tool without an EXTIO. This also impacts "
            "various functionality, e.g. sensor polling, light outputs\n");
        fprintf(
            stderr,
            "  -p3io     Explicit device path pointing to a P3IO device to "
            "open. If not provided, the device will be automatically scanned "
            "for.\n");
        fprintf(
            stderr,
            "  -extio    Explicitly set the COM port the EXTIO is connected "
            "to, e.g. COM4. If not provided COM1 is used.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Modes:\n");
        fprintf(
            stderr,
            "  scan: Scan for a connected P3IO device. Outputs the full device "
            "path to stdout if found.\n");
        fprintf(
            stderr,
            "  verp3io: Get and print the P3IO version information to "
            "stdout.\n");
        fprintf(stderr, "  init: Initialize the P3IO (\"DDR mode\").\n");
        fprintf(
            stderr,
            "  verddr: Get and print the DDR P3IO specific information to "
            "stdout.\n");
        fprintf(stderr, "  watchdogon: Turn the watchdog on.\n");
        fprintf(stderr, "  watchdogoff: Turn the watchdog off.\n");
        fprintf(
            stderr,
            "  cabtype: Get and print the cabinet type information to "
            "stdout.\n");
        fprintf(
            stderr, "  dipsw: Get and print the DIP switch state to stdout.\n");
        fprintf(
            stderr,
            "  videofreq: Get and print the detected video frequency to "
            "stdout.\n");
        fprintf(
            stderr,
            "  test: Run interactive test mode, executes a polling read/write "
            "loop to drive the IO and displays the current input states.\n");
        fprintf(stderr, "  lightoff: Turn all the lights off\n");
        fprintf(
            stderr,
            "  poll: Run a simple polling loop that polls data from the JAMMA "
            "endpoint with input state output.\n");
        fprintf(
            stderr,
            "  sensores: Run cycling individual sensores in a simple polling "
            "loop with input state ouptut.\n");

        return false;
    }

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            _log_level = CORE_LOG_BT_LOG_LEVEL_WARNING;
        } else if (!strcmp(argv[i], "-vv")) {
            _log_level = CORE_LOG_BT_LOG_LEVEL_INFO;
        } else if (!strcmp(argv[i], "-vvv")) {
            _log_level = CORE_LOG_BT_LOG_LEVEL_MISC;
        } else if (!strcmp(argv[i], "-noextio")) {
            _extio_enabled = false;
        } else if (!strcmp(argv[i], "-p3io")) {
            if (i + 1 < argc) {
                _p3io_device_path = argv[++i];
            } else {
                fprintf(stderr, "Missing value for -p3io argument\n");
            }
        } else if (!strcmp(argv[i], "-extio")) {
            if (i + 1 < argc) {
                _extio_com_port = argv[++i];
            } else {
                fprintf(stderr, "Missing value for -extio argument\n");
            }
        }
    }

    mode = argv[argc - 1];

    if (!strcmp(mode, "scan")) {
        _mode = MODE_SCAN;
    } else if (!strcmp(mode, "verp3io")) {
        _mode = MODE_VERP3IO;
    } else if (!strcmp(mode, "init")) {
        _mode = MODE_INIT;
    } else if (!strcmp(mode, "verddr")) {
        _mode = MODE_VERDDR;
    } else if (!strcmp(mode, "watchdogon")) {
        _mode = MODE_WATCHDOGON;
    } else if (!strcmp(mode, "watchdogoff")) {
        _mode = MODE_WATCHDOGOFF;
    } else if (!strcmp(mode, "cabtype")) {
        _mode = MODE_CABTYPE;
    } else if (!strcmp(mode, "dipsw")) {
        _mode = MODE_DIPSW;
    } else if (!strcmp(mode, "videofreq")) {
        _mode = MODE_VIDEOFREQ;
    } else if (!strcmp(mode, "test")) {
        _mode = MODE_TEST;
    } else if (!strcmp(mode, "lightoff")) {
        _mode = MODE_LIGHTOFF;
    } else if (!strcmp(mode, "poll")) {
        _mode = MODE_POLL;
    } else if (!strcmp(mode, "sensores")) {
        _mode = MODE_SENSORES;
    } else {
        fprintf(stderr, "Invalid mode '%s'", mode);
        return false;
    }

    return true;
}

static void _init_logging()
{
    core_log_bt_ext_impl_set();
    core_log_bt_ext_init_with_stderr();
    core_log_bt_level_set(_log_level);
}

static bool _mode_invalid(HANDLE handle)
{
    fprintf(stderr, "Invalid mode");
    return false;
}

static bool _mode_scan(HANDLE handle)
{
    HRESULT hr;
    char path[MAX_PATH];

    log_info("Scanning for p3io...");

    hr = p3iodrv_device_scan(path);

    if (FAILED(hr)) {
        log_warning("Cannot find a connected p3io: %lX", hr);
        return false;
    }

    log_info("Found p3io: %s", path);

    printf("%s\n", path);

    return true;
}

static bool _mode_verp3io(HANDLE handle)
{
    HRESULT hr;
    char version[P3IODRV_VERSION_MAX_LEN];

    memset(version, 0, P3IODRV_VERSION_MAX_LEN);

    hr = p3iodrv_device_read_version(handle, version);

    if (FAILED(hr)) {
        log_warning("Getting version failed: %lX", hr);
        return _close(handle);
    }

    log_info("Version (P3IO): %s", version);

    printf("%s\n", version);

    return true;
}

static bool _mode_init(HANDLE handle)
{
    HRESULT hr;

    hr = p3iodrv_ddr_init(handle);

    if (FAILED(hr)) {
        log_warning("Init failed: %lX", hr);
        return false;
    }

    log_info("Initialized");

    return true;
}

static bool _mode_verddr(HANDLE handle)
{
    HRESULT hr;
    char str[4];
    uint32_t major;
    uint32_t minor;
    uint32_t patch;

    hr = p3iodrv_ddr_get_version(handle, str, &major, &minor, &patch);

    if (FAILED(hr)) {
        log_warning("Getting DDR version failed: %lX", hr);
        return false;
    }

    log_info("Version (DDR): %s %d.%d.%d", str, major, minor, patch);

    printf("%s %d.%d.%d\n", str, major, minor, patch);

    return true;
}

static bool _mode_watchdogon(HANDLE handle)
{
    HRESULT hr;

    hr = p3iodrv_ddr_set_watchdog(handle, true);

    if (FAILED(hr)) {
        log_warning("Enabling watchdog failed: %lX", hr);
        return false;
    }

    log_info("Watchdog on");

    return true;
}

static bool _mode_watchdogoff(HANDLE handle)
{
    HRESULT hr;

    hr = p3iodrv_ddr_set_watchdog(handle, false);

    if (FAILED(hr)) {
        log_warning("Disabling watchdog failed: %lX", hr);
        return false;
    }

    log_info("Watchdog off");

    return true;
}

static bool _mode_cabtype(HANDLE handle)
{
    HRESULT hr;
    enum p3io_cab_type type;
    const char *type_str;

    hr = p3iodrv_ddr_get_cab_type(handle, &type);

    if (FAILED(hr)) {
        log_warning("Getting cab type failed: %lX", hr);
        return false;
    }

    log_info("Cabinet type: %d", type);

    switch (type) {
        case P3IO_CAB_TYPE_SD:
            type_str = "sd";
            break;

        case P3IO_CAB_TYPE_HD:
            type_str = "hd";
            break;

        default:
            type_str = "unknown";
            break;
    }

    printf("%s\n", type_str);

    return true;
}

static bool _mode_dipsw(HANDLE handle)
{
    HRESULT hr;
    uint8_t dip_sw;

    hr = p3iodrv_ddr_get_dipsw(handle, &dip_sw);

    if (FAILED(hr)) {
        log_warning("Getting dip switches failed: %lX", hr);
        return false;
    }

    log_info("Dip switches: 0x%X", dip_sw);

    printf("0x%X\n", dip_sw);

    return true;
}

static bool _mode_videofreq(HANDLE handle)
{
    HRESULT hr;
    enum p3io_video_freq video_freq;
    const char *video_freq_str;

    hr = p3iodrv_ddr_get_video_freq(handle, &video_freq);

    if (FAILED(hr)) {
        log_warning("Getting video freq failed: %lX", hr);
        return false;
    }

    log_info("Video freq: 0x%X", video_freq);

    switch (video_freq) {
        case P3IO_VIDEO_FREQ_15KHZ:
            video_freq_str = "15khz";
            break;

        case P3IO_VIDEO_FREQ_31KHZ:
            video_freq_str = "31khz";
            break;

        default:
            video_freq_str = "unknown";
            break;
    }

    printf("%s\n", video_freq_str);

    return true;
}

static bool _mode_test(HANDLE handle_p3io)
{
    HRESULT hr;
    HANDLE handle_extio;
    bool result;

    if (_extio_enabled) {
        hr = extiodrv_device_open(_extio_com_port, &handle_extio);

        if (FAILED(hr)) {
            log_warning(
                "Failed opening EXTIO on com port '%s': %lX",
                _extio_com_port,
                hr);
            return false;
        }
    } else {
        handle_extio = INVALID_HANDLE_VALUE;
    }

    result = mode_test_proc(handle_p3io, handle_extio);

    if (_extio_enabled) {
        hr = extiodrv_device_close(&handle_extio);

        if (FAILED(hr)) {
            log_warning("Failed closing EXTIO: %lX", hr);
            result = false;
        }
    }

    return result;
}

static bool _mode_lightoff(HANDLE handle_p3io)
{
    HRESULT hr;
    HANDLE handle_extio;

    struct p3io_ddr_output output;
    struct extiodrv_extio_pad_lights pad_lights[EXTIO_PAD_LIGHT_MAX_PLAYERS];
    bool neons;

    memset(&output, 0, sizeof(output));
    memset(&pad_lights, 0, sizeof(pad_lights));
    neons = false;

    if (_extio_enabled) {
        hr = extiodrv_device_open(_extio_com_port, &handle_extio);

        if (FAILED(hr)) {
            log_warning(
                "Failed opening EXTIO on com port '%s': %lX",
                _extio_com_port,
                hr);
            return false;
        }
    } else {
        handle_extio = INVALID_HANDLE_VALUE;
    }

    hr = p3iodrv_ddr_set_outputs(handle_p3io, &output);

    if (FAILED(hr)) {
        log_warning("Setting outputs failed: %lX", hr);
        return false;
    }

    if (_extio_enabled) {
        hr = extiodrv_extio_transfer(
            handle_extio,
            EXTIODRV_EXTIO_SENSOR_READ_MODE_ALL,
            pad_lights,
            neons);

        if (FAILED(hr)) {
            log_warning("extio transfer failed: %lX", hr);
            return false;
        }
    }

    if (_extio_enabled) {
        hr = extiodrv_device_close(&handle_extio);

        if (FAILED(hr)) {
            log_warning("Failed closing EXTIO: %lX", hr);
            return false;
        }
    }

    return true;
}

static bool _mode_poll(HANDLE handle_p3io)
{
    HRESULT hr;
    HANDLE handle_extio;

    struct p3io_ddr_jamma jamma;
    struct p3io_ddr_output output;
    struct extiodrv_extio_pad_lights pad_lights[EXTIO_PAD_LIGHT_MAX_PLAYERS];
    bool neons;

    uint32_t cnt;

    memset(&output, 0, sizeof(output));
    memset(&pad_lights, 0, sizeof(pad_lights));
    neons = false;

    if (_extio_enabled) {
        hr = extiodrv_device_open(_extio_com_port, &handle_extio);

        if (FAILED(hr)) {
            log_warning(
                "Failed opening EXTIO on com port '%s': %lX",
                _extio_com_port,
                hr);
            return false;
        }
    } else {
        handle_extio = INVALID_HANDLE_VALUE;
    }

    fprintf(
        stderr,
        ">>> Press enter to start endless polling loop. Press Escape to exit "
        "to exit polling loop <<<\n");

    if (getchar() != '\n') {
        return true;
    }

    cnt = 0;

    while ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) == 0) {
        hr = p3iodrv_ddr_set_outputs(handle_p3io, &output);

        if (FAILED(hr)) {
            log_warning("Setting outputs failed: %lX", hr);
            return false;
        }

        if (_extio_enabled) {
            hr = extiodrv_extio_transfer(
                handle_extio,
                EXTIODRV_EXTIO_SENSOR_READ_MODE_ALL,
                pad_lights,
                neons);

            if (FAILED(hr)) {
                log_warning("extio transfer failed: %lX", hr);
                return false;
            }
        }

        hr = p3iodrv_ddr_get_jamma(handle_p3io, &jamma);

        if (FAILED(hr)) {
            log_warning("Reading jamma failed: %lX", hr);
            return false;
        }

        printf(
            "%d: %08X %08X %08X\n",
            cnt,
            *((uint32_t *) &jamma) & 0xFFFFFF00,
            jamma.unused,
            jamma.unused2);

        cnt++;
    }

    if (_extio_enabled) {
        hr = extiodrv_device_close(&handle_extio);

        if (FAILED(hr)) {
            log_warning("Failed closing EXTIO: %lX", hr);
            return false;
        }
    }

    return true;
}

static bool _mode_sensores(HANDLE handle_p3io)
{
    HRESULT hr;
    HANDLE handle_extio;

    struct p3io_ddr_jamma jamma;
    struct p3io_ddr_output output;
    struct extiodrv_extio_pad_lights pad_lights[EXTIO_PAD_LIGHT_MAX_PLAYERS];
    bool neons;

    uint32_t cnt;
    uint8_t sensor_read;

    memset(&output, 0, sizeof(output));
    memset(&pad_lights, 0, sizeof(pad_lights));
    neons = false;

    if (_extio_enabled) {
        hr = extiodrv_device_open(_extio_com_port, &handle_extio);

        if (FAILED(hr)) {
            log_warning(
                "Failed opening EXTIO on com port '%s': %lX",
                _extio_com_port,
                hr);
            return false;
        }
    } else {
        handle_extio = INVALID_HANDLE_VALUE;
    }

    fprintf(
        stderr,
        ">>> Press enter to start endless sensor cycling loop. Press Escape to "
        "exit to exit sensor cycling loop <<<\n");

    if (getchar() != '\n') {
        return true;
    }

    cnt = 0;
    sensor_read = 0;

    while ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) == 0) {
        hr = p3iodrv_ddr_set_outputs(handle_p3io, &output);

        if (FAILED(hr)) {
            log_warning("Setting outputs failed: %lX", hr);
            return false;
        }

        if (_extio_enabled) {
            hr = extiodrv_extio_transfer(
                handle_extio, sensor_read + 1, pad_lights, neons);

            if (FAILED(hr)) {
                log_warning("extio transfer failed: %lX", hr);
                return false;
            }
        }

        hr = p3iodrv_ddr_get_jamma(handle_p3io, &jamma);

        if (FAILED(hr)) {
            log_warning("Reading jamma failed: %lX", hr);
            return false;
        }

        printf(
            "%d (%d): %08X %08X %08X\n",
            cnt,
            sensor_read,
            *((uint32_t *) &jamma) & 0xFFFFFF00,
            jamma.unused,
            jamma.unused2);

        cnt++;
        sensor_read++;

        if (sensor_read > 3) {
            sensor_read = 0;
        }
    }

    if (_extio_enabled) {
        hr = extiodrv_device_close(&handle_extio);

        if (FAILED(hr)) {
            log_warning("Failed closing EXTIO: %lX", hr);
            return false;
        }
    }

    return true;
}

int main(int argc, char **argv)
{
    HANDLE handle;
    bool result;

    if (!_process_cmd_args(argc, argv)) {
        return -1;
    }

    _init_logging();

    if (_mode != MODE_SCAN) {
        if (!_scan_and_open(&handle)) {
            result = false;
        } else {
            result = _mode_procs[_mode](handle);

            if (!_close(&handle)) {
                result = false;
            }
        }
    } else {
        result = _mode_procs[_mode](INVALID_HANDLE_VALUE);
    }

    return result ? 0 : -1;
}