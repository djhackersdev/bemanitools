#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"
#include "d3d9-monitor-check/input.h"
#include "d3d9-monitor-check/print-console.h"
#include "d3d9-monitor-check/refresh-rate-test.h"
#include "d3d9-monitor-check/response-time-test.h"
#include "d3d9-monitor-check/vsync-test.h"

static void _print_synopsis()
{
    printfln_err("D3D9 monitor check tool command line mode");
    printfln_err("");
    printfln_err("Usage:");
    printfln_err("  d3d9-monitor-check cmdline <command> <args>");
    printfln_err("");
    printfln_err("Available commands:");
    printfln_err("  adapter: Query adapter information");
    printfln_err("  modes: Query adapter modes. Use this to get supported mandatory parameters for width, height and refresh rate for the tests.");
    printfln_err("");
    printfln_err("  refresh-rate-test <width> <height> <refresh_rate> [--warm-up-secs n] [--total-secs n] [--results-timeout-secs n] [--windowed] [--vsync-off]: Run a display refresh rate test (i.e. IIDX monitor check).");
    printfln_err("    width: Width of the rendering resolution to run the test at");
    printfln_err("    height: Height of the rendering resolution to run the test at");
    printfln_err("    refresh_rate: Target refresh rate to run the test at");
    printfln_err("    warm-up-secs: Optional. Number of seconds to warm-up before executing the main run that counts towards the measurement results");
    printfln_err("    total-secs: Optional. Total number of seconds to run the test for that count towards the measurement results");
    printfln_err("    results-timeout-secs: Optional. Number of seconds to display final result after the test before exiting");
    printfln_err("    windowed: Optional. Run the test in windowed mode (not recommended)");
    printfln_err("    vsync-off: Optional. Run the test with vsync off (not recommended)");
    printfln_err("");
    printfln_err("  response-time-test <width> <height> <refresh_rate> [--total-secs n] [--windowed] [--vsync-off]: Run a test to visually inspect the display's response times.");
    printfln_err("    width: Width of the rendering resolution to run the test at");
    printfln_err("    height: Height of the rendering resolution to run the test at");
    printfln_err("    refresh_rate: Target refresh rate to run the test at");
    printfln_err("    total-secs: Optional. Total number of seconds to run the test for that count towards the measurement results");
    printfln_err("    windowed: Optional. Run the test in windowed mode (not recommended)");
    printfln_err("    vsync-off: Optional. Run the test with vsync off (not recommended)");
    printfln_err("");
    printfln_err("  vsync-test <width> <height> <refresh_rate> [--total-secs n] [--windowed]: Run a test to visually inspect the display's vsync behavior.");
    printfln_err("    width: Width of the rendering resolution to run the test at");
    printfln_err("    height: Height of the rendering resolution to run the test at");
    printfln_err("    refresh_rate: Target refresh rate to run the test at");
    printfln_err("    total-secs: Optional. Total number of seconds to run the test for that count towards the measurement results");
    printfln_err("    windowed: Optional. Run the test in windowed mode (not recommended)");
}

static bool _adapter()
{
    D3DADAPTER_IDENTIFIER9 identifier;

    if (!gfx_adapter_info_get(&identifier)) {
        return false;
    }

    printfln_out("Driver: %s", identifier.Driver);
    printfln_out("Description: %s", identifier.Description);
    printfln_out("DeviceName: %s", identifier.DeviceName);
#ifdef _WIN32
    printfln_out("DriverVersion: %lld", identifier.DriverVersion.QuadPart);
#else
    printfln_out("DriverVersion: %lu.%lu", identifier.DriverVersionHighPart, identifier.DriverVersionLowPart);
#endif
    printfln_out("VendorId: %lu", identifier.VendorId);
    printfln_out("DeviceId: %lu", identifier.DeviceId);
    printfln_out("SubSysId: %lu", identifier.SubSysId);
    printfln_out("Revision: %lu", identifier.Revision);
    printfln_out("DeviceIdentifier: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        identifier.DeviceIdentifier.Data1,
        identifier.DeviceIdentifier.Data2,
        identifier.DeviceIdentifier.Data3,
        identifier.DeviceIdentifier.Data4[0],
        identifier.DeviceIdentifier.Data4[1],
        identifier.DeviceIdentifier.Data4[2],
        identifier.DeviceIdentifier.Data4[3],
        identifier.DeviceIdentifier.Data4[4],
        identifier.DeviceIdentifier.Data4[5],
        identifier.DeviceIdentifier.Data4[6],
        identifier.DeviceIdentifier.Data4[7]);
    printfln_out("WHQLLevel: %lu", identifier.WHQLLevel);

    return true;
}

static bool _modes()
{
    gfx_adapter_modes_t modes;

    if (!gfx_adapter_modes_get(&modes)) {
        return false;
    }
    
    for (uint32_t i = 0; i < modes.count; i++) {
        printfln_out("%d: %d x %d @ %d hz", i, modes.modes[i].Width, modes.modes[i].Height, modes.modes[i].RefreshRate);
    }

    return true;
}

static bool _refresh_rate_test(
    uint32_t width,
    uint32_t height,
    uint32_t refresh_rate,
    uint32_t warm_up_frame_count,
    uint32_t sample_frame_count,
    uint32_t result_timeout_frame_count,
    bool windowed,
    bool vsync)
{
    gfx_t *gfx;
    input_t *input;
    refresh_rate_test_t *test;
    gfx_info_t gfx_info;
    refresh_rate_test_results_t results;
    uint32_t results_timeout_seconds;
    
    input_init(&input);

    if (!gfx_init(width, height, refresh_rate, windowed, vsync, &gfx)) {
        input_fini(input);
        return false;
    }

    if (!refresh_rate_test_init(gfx, warm_up_frame_count, sample_frame_count, &test)) {
        input_fini(input);
        gfx_fini(gfx);
        return false;
    }

    do {
        input_update(input);

        if (input_key_esc_pushed(input)) {
            break;
        }
    } while (refresh_rate_test_frame_update(test) && 
        gfx_last_frame_count_get(gfx) < warm_up_frame_count + sample_frame_count);

    results_timeout_seconds = result_timeout_frame_count / refresh_rate;

    // Display final results
    for (uint32_t i = 0; i < result_timeout_frame_count; i++) {
        input_update(input);

        if (input_key_esc_pushed(input)) {
            break;
        }

        if (!refresh_rate_test_results_frame_update(test, results_timeout_seconds)) {
            break;
        }
    }
    
    gfx_info_get(gfx, &gfx_info);
    refresh_rate_test_results_get(test, &results);

    printfln_err("Final results");
    printfln_out("GPU: %s", gfx_info.adapter_identifier);
    printfln_out("Spec: %d x %d @ %d hz, %s, vsync %s", gfx_info.width, gfx_info.height, gfx_info.refresh_rate, 
        gfx_info.windowed ? "windowed" : "fullscreen", gfx_info.vsync ? "on" : "off");
    printfln_out("Total warm-up frame count: %d", results.total_warm_up_frame_count);
    printfln_out("Total sample frame count: %d", results.total_sample_frame_count);
    printfln_out("Avg frame time (ms): %.3f", results.avg_frame_time_ms);
    printfln_out("Avg refresh rate (hz): %.3f", results.avg_refresh_rate_hz);

    refresh_rate_test_fini(test);
    input_fini(input);
    gfx_fini(gfx);

    return true;
}

static bool _response_time_test(
    uint32_t width,
    uint32_t height,
    uint32_t refresh_rate,
    uint32_t total_frame_count,
    bool windowed,
    bool vsync)
{
    gfx_t *gfx;
    input_t *input;
    response_time_test_t *test;
    
    input_init(&input);

    // Force vsync on, off option doesn't make sense for this test
    if (!gfx_init(width, height, refresh_rate, windowed, vsync, &gfx)) {
        input_fini(input);
        return false;
    }

    if (!response_time_test_init(gfx, &test)) {
        input_fini(input);
        gfx_fini(gfx);
        return false;
    }

    do {
        input_update(input);

        if (input_key_esc_pushed(input)) {
            break;
        }
    } while (response_time_test_frame_update(test) && 
        gfx_last_frame_count_get(gfx) < total_frame_count);

    response_time_test_fini(test);
    input_fini(input);
    gfx_fini(gfx);

    return true;
}

static bool _vsync_test(
    uint32_t width,
    uint32_t height,
    uint32_t refresh_rate,
    uint32_t total_frame_count,
    bool windowed)
{
    gfx_t *gfx;
    input_t *input;
    vsync_test_t *test;
    
    input_init(&input);

    // Force vsync on, off option doesn't make sense for this test
    if (!gfx_init(width, height, refresh_rate, windowed, true, &gfx)) {
        input_fini(input);
        return false;
    }

    if (!vsync_test_init(gfx, &test)) {
        input_fini(input);
        gfx_fini(gfx);
        return false;
    }

    do {
        input_update(input);

        if (input_key_esc_pushed(input)) {
            break;
        }
    } while (vsync_test_frame_update(test) && 
        gfx_last_frame_count_get(gfx) < total_frame_count);

    vsync_test_fini(test);
    input_fini(input);
    gfx_fini(gfx);

    return true;
}

static bool _cmd_refresh_rate_test(int argc, char **argv)
{
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
    uint32_t warm_up_seconds;
    uint32_t sample_seconds;
    uint32_t results_timeout_seconds;
    bool windowed;
    bool vsync;

    uint32_t total_warm_up_frame_count;
    uint32_t total_sample_frame_count;
    uint32_t result_timeout_frame_count;

    if (argc < 3) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    width = atoi(argv[0]);

    if (width == 0 || width > 16384) {
        _print_synopsis();
        printfln_err("ERROR: Invalid width: %d", width);
        return false;
    }

    height = atoi(argv[1]);

    if (height == 0 || height > 16384) {
        _print_synopsis();
        printfln_err("ERROR: Invalid height: %d", height);
        return false;
    }

    refresh_rate = atoi(argv[2]);

    if (refresh_rate == 0 || refresh_rate > 1000) {
        _print_synopsis();
        printfln_err("ERROR: Invalid refresh rate: %d", refresh_rate);
        return false;
    }

    // Sane defaults
    warm_up_seconds = 10;
    sample_seconds = 20;
    results_timeout_seconds = 5;
    windowed = false;
    vsync = true;

    for (int i = 3; i < argc; i++) {
        if (!strcmp(argv[i], "--warm-up-secs")) {
            if (i + 1 < argc) {
                warm_up_seconds = atoi(argv[++i]); 

                if (warm_up_seconds == 0) {
                    _print_synopsis();
                    printfln_err("ERROR: Invalid warm-up seconds: %d", warm_up_seconds);
                    return false;
                }
            } else {
                _print_synopsis();
                printfln_err("ERROR: Missing argument for --warm-up-secs");
                return false;
            }
        } else if (!strcmp(argv[i], "--sample-secs")) {
            if (i + 1 < argc) {
                sample_seconds = atoi(argv[++i]);
                
                if (sample_seconds == 0) {
                    _print_synopsis();
                    printfln_err("ERROR: Invalid sample seconds: %d", sample_seconds);
                    return false;
                }
            } else {
                _print_synopsis();
                printfln_err("ERROR: Missing argument for --total-secs");
                return false;
            }
        } else if (!strcmp(argv[i], "--results-timeout-secs")) {
            if (i + 1 < argc) {
                results_timeout_seconds = atoi(argv[++i]);
            } else {
                _print_synopsis();
                printfln_err("ERROR: Missing argument for --results-timeout-secs");
            }
        } else if (!strcmp(argv[i], "--windowed")) {
            windowed = true;
        } else if (!strcmp(argv[i], "--vsync-off")) {
            vsync = false;
        }
    }

    total_warm_up_frame_count = warm_up_seconds * refresh_rate;
    total_sample_frame_count = sample_seconds * refresh_rate;
    result_timeout_frame_count = results_timeout_seconds * refresh_rate;

    return _refresh_rate_test(
        width,
        height,
        refresh_rate,
        total_warm_up_frame_count,
        total_sample_frame_count,
        result_timeout_frame_count,
        windowed,
        vsync);
}

static bool _cmd_response_time_test(int argc, char **argv)
{
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
    uint32_t total_seconds;
    bool windowed;
    bool vsync;

    uint32_t total_frame_count;

    if (argc < 3) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    width = atoi(argv[0]);

    if (width == 0 || width > 16384) {
        _print_synopsis();
        printfln_err("ERROR: Invalid width: %d", width);
        return false;
    }

    height = atoi(argv[1]);

    if (height == 0 || height > 16384) {
        _print_synopsis();
        printfln_err("ERROR: Invalid height: %d", height);
        return false;
    }

    refresh_rate = atoi(argv[2]);

    if (refresh_rate == 0 || refresh_rate > 1000) {
        _print_synopsis();
        printfln_err("ERROR: Invalid refresh rate: %d", refresh_rate);
        return false;
    }

    // Sane defaults
    total_seconds = 30;
    windowed = false;
    vsync = true;
    for (int i = 3; i < argc; i++) {
        if (!strcmp(argv[i], "--total-secs")) {
            if (i + 1 < argc) {
                total_seconds = atoi(argv[++i]);
                
                if (total_seconds == 0) {
                    _print_synopsis();
                    printfln_err("ERROR: Invalid total seconds: %d", total_seconds);
                    return false;
                }
            } else {
                _print_synopsis();
                printfln_err("ERROR: Missing argument for --total-secs");
                return false;
            }
        } else if (!strcmp(argv[i], "--windowed")) {
            windowed = true;
        } else if (!strcmp(argv[i], "--vsync-off")) {
            vsync = false;
        }
    }

    total_frame_count = total_seconds * refresh_rate;

    return _response_time_test(width, height, refresh_rate, total_frame_count, windowed, vsync);
}

static bool _cmd_vsync_test(int argc, char **argv)
{
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
    uint32_t total_seconds;
    bool windowed;

    uint32_t total_frame_count;

    if (argc < 3) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    width = atoi(argv[0]);

    if (width == 0 || width > 16384) {
        _print_synopsis();
        printfln_err("ERROR: Invalid width: %d", width);
        return false;
    }

    height = atoi(argv[1]);

    if (height == 0 || height > 16384) {
        _print_synopsis();
        printfln_err("ERROR: Invalid height: %d", height);
        return false;
    }

    refresh_rate = atoi(argv[2]);

    if (refresh_rate == 0 || refresh_rate > 1000) {
        _print_synopsis();
        printfln_err("ERROR: Invalid refresh rate: %d", refresh_rate);
        return false;
    }

    // Sane defaults
    total_seconds = 30;
    windowed = false;

    for (int i = 3; i < argc; i++) {
        if (!strcmp(argv[i], "--total-secs")) {
            if (i + 1 < argc) {
                total_seconds = atoi(argv[++i]);
                
                if (total_seconds == 0) {
                    _print_synopsis();
                    printfln_err("ERROR: Invalid total seconds: %d", total_seconds);
                    return false;
                }
            } else {
                _print_synopsis();
                printfln_err("ERROR: Missing argument for --total-secs");
                return false;
            }
        } else if (!strcmp(argv[i], "--windowed")) {
            windowed = true;
        }
    }

    total_frame_count = total_seconds * refresh_rate;

    return _vsync_test(width, height, refresh_rate, total_frame_count, windowed);
}

bool cmdline_main(int argc, char **argv)
{
    const char *command;

    if (argc < 1) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    command = argv[0];

    if (!strcmp(command, "adapter")) {
        return _adapter();
    } else if (!strcmp(command, "modes")) {
        return _modes();
    } else if (!strcmp(command, "refresh-rate-test")) {
        return _cmd_refresh_rate_test(argc - 1, argv + 1);
    } else if (!strcmp(command, "response-time-test")) {
        return _cmd_response_time_test(argc - 1, argv + 1);
    } else if (!strcmp(command, "vsync-test")) {
        return _cmd_vsync_test(argc - 1, argv + 1);
    } else {
        _print_synopsis(argv[0]);
        printfln_err("ERROR: Unknown command: %s", command);
        return false;
    }
}