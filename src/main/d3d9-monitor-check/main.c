#include <windows.h>

#include <d3d9.h>
#include <d3dx9core.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "util/time.h"
#include "util/winerr.h"

#define printf_out(fmt, ...) \
    fprintf(stdout, fmt, ##__VA_ARGS__)
#define printf_err(fmt, ...) \
    fprintf(stderr, fmt, ##__VA_ARGS__)
#define printfln_out(fmt, ...) \
    fprintf(stdout, fmt "\n", ##__VA_ARGS__)
#define printfln_err(fmt, ...) \
    fprintf(stderr, fmt "\n", ##__VA_ARGS__)

#define printfln_winerr(fmt, ...) \
    char *winerr = util_winerr_format_last_error_code(); \
    fprintf(stderr, fmt ": %s\n", ##__VA_ARGS__, winerr); \
    free(winerr);

static const D3DFORMAT _d3dformat = D3DFMT_X8R8G8B8;

static void _print_synopsis()
{
    printfln_err("D3D9 monitor check");
    printfln_err("");
    printfln_err("Improved open source re-implementation of IIDX's infamous \"monitor check\" screen");
    printfln_err("Run a bare D3D9 render loop to measure the refresh rate of the current GPU + monitor configuration");
    printfln_err("");
    printfln_err("Usage:");
    printfln_err("  d3d9-monitor-check <command> <args>");
    printfln_err("");
    printfln_err("Available commands:");
    printfln_err("  adapter: Query adapter information");
    printfln_err("  modes: Query adapter modes");
    printfln_err("  run <width> <height> <refresh_rate> [--warm-up-secs n] [--total-secs n] [--windowed] [--vsync-off]: Run the monitor check. Ensure that the mandatory parameters for width, height and refresh rate are values that are supported by the adapter's mode. Use the \"modes\" subcommand to get a list of supported modes.");
    printfln_err("    width: Width of the rendering resolution to run the test at");
    printfln_err("    height: Height of the rendering resolution to run the test at");
    printfln_err("    refresh_rate: Target refresh rate to run the test at");
    printfln_err("    warm-up-secs: Optional. Number of seconds to warm-up before executing the main run that counts towards the measurement results");
    printfln_err("    total-secs: Optional. Total number of seconds to run the test for that count towards the measurement results");
    printfln_err("    windowed: Optional. Run the test in windowed mode (not recommended)");
    printfln_err("    vsync-off: Optional. Run the test with vsync off (not recommended)");
}

static bool _create_d3d_context(IDirect3D9 **d3d)
{
    // Initialize D3D
    *d3d = Direct3DCreate9(D3D_SDK_VERSION);

    if (!*d3d) {
        printfln_winerr("Creating d3d context failed");
        return false;
    }

    return true;
}

static bool _query_adapter_identifier(IDirect3D9 *d3d, D3DADAPTER_IDENTIFIER9 *identifier)
{
    HRESULT hr;

    hr = IDirect3D9_GetAdapterIdentifier(d3d, D3DADAPTER_DEFAULT, 0, identifier);

    if (hr != D3D_OK) {
        printfln_winerr("GetAdapterIdentifier failed");
        return false;
    }

    return true;
}

static bool _create_window(uint32_t width, uint32_t height, HWND *hwnd)
{
    WNDCLASSEX wc;

    memset(&wc, 0, sizeof(wc));

    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "D3D9MonitorCheck";

    RegisterClassExA(&wc);

    // Create window
    *hwnd = CreateWindowA(
        wc.lpszClassName,
        "D3D9 Monitor Check",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        NULL,
        NULL,
        wc.hInstance,
        NULL);

    if (!*hwnd) {
        printfln_winerr("Failed to create window");
        return false;
    }

    return true;
}

static bool _create_d3d_device(
    HWND hwnd,
    IDirect3D9 *d3d,
    uint32_t width,
    uint32_t height,
    uint32_t refresh_rate,
    bool windowed,
    bool vsync_off,
    IDirect3DDevice9 **device)
{
    D3DPRESENT_PARAMETERS pp;
    HRESULT hr;

    memset(&pp, 0, sizeof(pp));

    if (windowed) {
        ShowWindow(hwnd, SW_SHOW);

        pp.Windowed = TRUE;
        pp.FullScreen_RefreshRateInHz = 0;
    } else {
        ShowCursor(FALSE);

        pp.Windowed = FALSE;
        pp.FullScreen_RefreshRateInHz = refresh_rate;
    }

    if (vsync_off) {
        pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    } else {
        pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    }

    pp.BackBufferWidth = width;
    pp.BackBufferHeight = height;
    pp.BackBufferFormat = _d3dformat;
    pp.BackBufferCount = 2;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.MultiSampleQuality = 0;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = hwnd;
    pp.EnableAutoDepthStencil = TRUE;
    pp.AutoDepthStencilFormat = D3DFMT_D16;
    pp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    // Create D3D device
    hr = IDirect3D9_CreateDevice(
        d3d,
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &pp,
        device);

    if (hr != D3D_OK) {
        printfln_winerr("Creating d3d device failed");
        return false;
    }

    return true;
}

static uint32_t _get_font_height(uint32_t resolution_height)
{
    // Default size for 480p
    return (uint32_t) (20.0f * resolution_height / 480.0f);
}

static uint32_t _get_text_offset_x(uint32_t resolution_width)
{
    // Default offset for 480p
    return (uint32_t) (20.0f * resolution_width / 480.0f);
}

static uint32_t _get_text_offset_y(uint32_t resolution_height, uint32_t font_height)
{
    // Default offset for 480p
    return (uint32_t) (font_height + 10 * (resolution_height / 640.0f));
}

static bool _create_font(IDirect3DDevice9 *device, uint32_t font_height, ID3DXFont **font)
{
    HRESULT hr;

    hr = D3DXCreateFont(device, font_height, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        "Arial", font);

    if (hr != D3D_OK) {
        printfln_winerr("Creating font failed");
        return false;
    }

    return true;
}

static void _draw_text(IDirect3DDevice9 *device, ID3DXFont *font, uint32_t font_height, int x, int y, const char *fmt, ...)
{
    va_list args;
    char text[1024];
    RECT rect;

    va_start(args, fmt);
    vsprintf(text, fmt, args);
    va_end(args);

    rect.left = x;
    rect.top = y;
    // Base width of 300 is based on 480p
    rect.right = x + (480 * (font_height / 20.0f));
    rect.bottom = y + font_height;

    ID3DXFont_DrawText(font, NULL, text, -1, &rect, DT_LEFT | DT_TOP, D3DCOLOR_XRGB(255, 255, 255));
}

static bool _is_esc_key_pressed()
{
    return GetAsyncKeyState(VK_ESCAPE) & 0x8000;
}

static bool _is_esc_key_released()
{
    return !(GetAsyncKeyState(VK_ESCAPE) & 0x8000);
}

static bool _adapter()
{
    IDirect3D9 *d3d;
    D3DADAPTER_IDENTIFIER9 identifier;

    if (!_create_d3d_context(&d3d)) {
        return false;
    }
    
    if (!_query_adapter_identifier(d3d, &identifier)) {
        IDirect3D9_Release(d3d);
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
    
    IDirect3D9_Release(d3d);

    return true;
}

static bool _modes()
{
    IDirect3D9 *d3d;
    HRESULT hr;
    UINT mode_count;
    D3DDISPLAYMODE mode;

    memset(&mode, 0, sizeof(D3DDISPLAYMODE));

    if (!_create_d3d_context(&d3d)) {
        return false;
    }

    mode_count = IDirect3D9_GetAdapterModeCount(d3d, D3DADAPTER_DEFAULT, _d3dformat);

    printfln_err("Available adapter modes (total %d)", mode_count);
    printfln_err("Mode index: width x height @ refresh rate");

    for (UINT i = 0; i < mode_count; i++) {
        hr = IDirect3D9_EnumAdapterModes(d3d, D3DADAPTER_DEFAULT, _d3dformat, i, &mode);

        if (hr != D3D_OK) {
            printfln_winerr("EnumAdapterMode index %d failed", i);
            IDirect3D9_Release(d3d);
            return false;
        }

        printfln_out("%d: %d x %d @ %d hz", i, mode.Width, mode.Height, mode.RefreshRate);
    }

    IDirect3D9_Release(d3d);

    return true;
}

static bool _run(uint32_t width, uint32_t height, uint32_t refresh_rate, uint32_t total_warm_up_frame_count, uint32_t total_frame_count, bool windowed, bool vsync_off)
{
    HWND hwnd;
    IDirect3D9 *d3d;
    D3DADAPTER_IDENTIFIER9 identifier;
    IDirect3DDevice9 *device;
    uint32_t font_height;
    ID3DXFont *font;
    uint32_t text_offset_x;
    uint32_t text_offset_y;

    MSG msg;
    bool exit_loop;
    bool warm_up_done;
    uint32_t warm_up_frame_count;
    uint32_t frame_count;
    uint64_t start_time;
    uint64_t end_time;
    uint64_t elapsed_us;
    uint64_t total_elapsed_us;

    printfln_err("Creating d3d context ...");

    if (!_create_d3d_context(&d3d)) {
        return false;
    }
    
    printfln_err("Querying adapter identifier ...");

    if (!_query_adapter_identifier(d3d, &identifier)) {
        IDirect3D9_Release(d3d);
        return false;
    }

    printfln_err("Adapter:");
    printfln_err("Driver: %s", identifier.Driver);
    printfln_err("Description: %s", identifier.Description);
    printfln_err("DeviceName: %s", identifier.DeviceName);
#ifdef _WIN32
    printfln_err("DriverVersion: %lld", identifier.DriverVersion.QuadPart);
#else
    printfln_err("DriverVersion: %lu.%lu", identifier.DriverVersionHighPart, identifier.DriverVersionLowPart);
#endif

    printfln_err("Creating window with %dx%d ...", width, height);

    if (!_create_window(width, height, &hwnd)) {
        IDirect3D9_Release(d3d);
        return false;
    }

    printfln_err("Creating d3d device %d x %d @ %d hz %s vsync %s ...",
        width,
        height,
        refresh_rate,
        windowed ? "windowed" : "fullscreen",
        vsync_off ? "off" : "on");

    if (!_create_d3d_device(
            hwnd,
            d3d,
            width,
            height,
            refresh_rate,
            windowed,
            vsync_off,
            &device)) {
        IDirect3D9_Release(d3d);
        DestroyWindow(hwnd);
        return false;
    }

    printfln_err("Creating font ...");

    font_height = _get_font_height(height);

    if (!_create_font(device, font_height, &font)) {
        IDirect3DDevice9_Release(device);
        IDirect3D9_Release(d3d);
        DestroyWindow(hwnd);
        return false;
    }

    text_offset_x = _get_text_offset_x(width);
    text_offset_y = _get_text_offset_y(height, font_height);

    // ---------------------------------------------------------------------------------------------

    exit_loop = false;
    warm_up_done = false;

    warm_up_frame_count = 0;
    frame_count = 0;

    elapsed_us = 0;
    total_elapsed_us = 0;

    printfln_err("Warm-up for %d frames ...", total_warm_up_frame_count);

    start_time = time_get_counter();

    while (warm_up_frame_count + frame_count < total_warm_up_frame_count + total_frame_count) {
        // reset when warm-up is done
        if (warm_up_frame_count >= total_warm_up_frame_count && !warm_up_done) {
            warm_up_done = true;
            total_elapsed_us = 0;
            printfln_err("Warm-up finished");
            printfln_err("Running test for %d frames ...", total_frame_count);
        }

        // Required to not make windows think we are stuck and not responding
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                exit_loop = true;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (exit_loop) {
            break;
        }

        if (_is_esc_key_pressed()) {
            // Avoid multi triggering with further key evaluations
            while (!_is_esc_key_released()) {
                Sleep(10);
            }

            exit_loop = true;
            break;
        }

        IDirect3DDevice9_Clear(
            device, 
            0, 
            NULL, 
            D3DCLEAR_TARGET,
            D3DCOLOR_XRGB(0, 0, 0), 
            1.0f, 
            0);     
        IDirect3DDevice9_BeginScene(device);

        _draw_text(device, font, font_height, text_offset_x, text_offset_y, "D3D9 Monitor Check");
        _draw_text(device, font, font_height, text_offset_x, text_offset_y * 3,
            "GPU: %s", identifier.Description);
        _draw_text(device, font, font_height, text_offset_x, text_offset_y * 4,
            "Spec: %d x %d @ %d hz, %s, vsync %s", width, height, refresh_rate, 
            windowed ? "windowed" : "fullscreen", vsync_off ? "off" : "on");

        if (warm_up_frame_count < total_warm_up_frame_count) {
            // First frame won't have any data available causing division by zero in the stats
            if (warm_up_frame_count != 0) {
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 6, "Status: Warm-up in progress ...");

                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 7, 
                    "Frame: %d / %d", warm_up_frame_count, total_warm_up_frame_count);
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 8, 
                    "Last frame time: %.3f ms", elapsed_us / 1000.0f);
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 9, 
                    "Avg frame time: %.3f ms", total_elapsed_us / warm_up_frame_count / 1000.0f);
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 10, 
                    "Last refresh rate: %.3f Hz", 1000.0f / (elapsed_us / 1000.0f));
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 11, 
                    "Avg refresh rate: %.3f Hz", 1000.0f / (total_elapsed_us / warm_up_frame_count / 1000.0f));
            }
        } else {
            // First frame won't have any data available causing division by zero in the stats
            if (frame_count != 0) {
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 6, "Status: Measuring in progress ...");

                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 7, 
                    "Frame: %d / %d", frame_count, total_frame_count);
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 8, 
                    "Last frame time: %.3f ms", elapsed_us / 1000.0f);
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 9, 
                    "Avg frame time: %.3f ms", total_elapsed_us / frame_count / 1000.0f);
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 10, 
                    "Last refresh rate: %.3f Hz", 1000.0f / (elapsed_us / 1000.0f));
                _draw_text(device, font, font_height, text_offset_x, text_offset_y * 11, 
                    "Avg refresh rate: %.3f Hz", 1000.0f / (total_elapsed_us / frame_count / 1000.0f));
            }
        }

        _draw_text(device, font, font_height, text_offset_x, text_offset_y * 13, "Press ESC to exit early");

        IDirect3DDevice9_EndScene(device);
        IDirect3DDevice9_Present(device, NULL, NULL, NULL, NULL);

        end_time = time_get_counter();
        elapsed_us = time_get_elapsed_us(end_time - start_time);
        start_time = end_time;
        total_elapsed_us += elapsed_us;

        if (warm_up_frame_count < total_warm_up_frame_count) {
            warm_up_frame_count++;
        } else {
            frame_count++;
        }
    }

    // ---------------------------------------------------------------------------------------------

    printfln_err("Running test finished");

    IDirect3DDevice9_Clear(
        device, 
        0, 
        NULL, 
        D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(0, 0, 0), 
        1.0f, 
        0);     
    IDirect3DDevice9_BeginScene(device);

    _draw_text(device, font, font_height, text_offset_x, text_offset_y, "D3D9 Monitor Check");
    _draw_text(device, font, font_height, text_offset_x, text_offset_y * 3,
            "GPU: %s", identifier.Description);
    _draw_text(device, font, font_height, text_offset_x, text_offset_y * 4,
            "Spec: %d x %d @ %d hz, %s, vsync %s", width, height, refresh_rate, 
            windowed ? "windowed" : "fullscreen", vsync_off ? "off" : "on");

    if (exit_loop) {
        _draw_text(device, font, font_height, text_offset_x, text_offset_y * 6, "Status: Exited early");
    } else {
        _draw_text(device, font, font_height, text_offset_x, text_offset_y * 6, "Status: Finished");
    }

    _draw_text(device, font, font_height, text_offset_x, text_offset_y * 7, 
        "Total warm-up frame count: %d", warm_up_frame_count);
    _draw_text(device, font, font_height, text_offset_x, text_offset_y * 8, 
        "Total sample frame count: %d", frame_count);
    _draw_text(device, font, font_height, text_offset_x, text_offset_y * 9, 
        "Avg frame time: %.3f ms", total_elapsed_us > 0 && frame_count > 0 ? total_elapsed_us / frame_count / 1000.0f : 0);
    _draw_text(device, font, font_height, text_offset_x, text_offset_y * 10, 
        "Avg refresh rate: %.3f Hz", total_elapsed_us > 0 && frame_count > 0 ? 1000.0f / (total_elapsed_us / frame_count / 1000.0f) : 0);

    _draw_text(device, font, font_height, text_offset_x, text_offset_y * 12, "Exiting in 5 seconds ...");
    _draw_text(device, font, font_height, text_offset_x, text_offset_y * 13, "Press ESC to exit immediately");

    IDirect3DDevice9_EndScene(device);
    IDirect3DDevice9_Present(device, NULL, NULL, NULL, NULL);

    exit_loop = false;

    for (uint32_t i = 0; i < 5000 / 10; i++) {
        // Required to not make windows think we are stuck and not responding
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                exit_loop = true;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (exit_loop) {
            break;
        }

        // Allow quick exit
        if (_is_esc_key_pressed()) {
            // Avoid multi triggering with further key evaluations
            while (!_is_esc_key_released()) {
                Sleep(10);
            }

            exit_loop = true;
            break;
        }

        Sleep(10);
    }

    // ---------------------------------------------------------------------------------------------

    printfln_err("Final results");
    printfln_out("GPU: %s", identifier.Description);
    printfln_out("Spec: %d x %d @ %d hz, %s, vsync %s", width, height, refresh_rate, 
        windowed ? "windowed" : "fullscreen", vsync_off ? "off" : "on");
    printfln_out("Avg frame time (ms): %.3f", total_elapsed_us > 0 && frame_count > 0 ? total_elapsed_us / frame_count / 1000.0f : 0);
    printfln_out("Avg refresh rate (hz): %.3f", total_elapsed_us > 0 && frame_count > 0 ? 1000.0f / (total_elapsed_us / frame_count / 1000.0f) : 0);

    ID3DXFont_Release(font);
    IDirect3DDevice9_Release(device);
    IDirect3D9_Release(d3d);
    DestroyWindow(hwnd);

    return true;
}

static bool _cmd_adapter()
{
    return _adapter();
}

static bool _cmd_modes()
{
    return _modes();
}

static bool _cmd_run(int argc, char **argv)
{
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
    uint32_t warm_up_seconds;
    uint32_t total_seconds;
    bool windowed;
    bool vsync_off;

    uint32_t total_warm_up_frame_count;
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
    warm_up_seconds = 10;
    total_seconds = 20;
    windowed = false;
    vsync_off = false;

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
        } else if (!strcmp(argv[i], "--total-secs")) {
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
            vsync_off = true;
        }
    }

    total_warm_up_frame_count = warm_up_seconds * refresh_rate;
    total_frame_count = total_seconds * refresh_rate;

    return _run(width, height, refresh_rate, total_warm_up_frame_count, total_frame_count, windowed, vsync_off);
}

int main(int argc, char **argv)
{
    const char *command;

    if (argc < 2) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return 1;
    }

    command = argv[1];

    if (!strcmp(command, "adapter")) {
        if (!_cmd_adapter(argc - 2, argv + 2)) {
            return 1;
        }
    } else if (!strcmp(command, "modes")) {
        if (!_cmd_modes(argc - 2, argv + 2)) {
            return 1;
        }
    } else if (!strcmp(command, "run")) {
        if (!_cmd_run(argc - 2, argv + 2)) {
            return 1;
        }
    } else {
        _print_synopsis(argv[0]);
        printfln_err("ERROR: Unknown command: %s", command);
        return 1;
    }

    return 0;
}
