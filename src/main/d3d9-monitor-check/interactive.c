#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"
#include "d3d9-monitor-check/input.h"
#include "d3d9-monitor-check/menu.h"
#include "d3d9-monitor-check/print-console.h"
#include "d3d9-monitor-check/refresh-rate-test.h"
#include "d3d9-monitor-check/response-time-test.h"
#include "d3d9-monitor-check/vsync-test.h"

typedef enum INTERACTIVE_SCREEN {
    INTERACTIVE_SCREEN_MENU = 0,
    INTERACTIVE_SCREEN_REFRESH_RATE_TEST = 1,
    INTERACTIVE_SCREEN_RESPONSE_TIME_TEST = 2,
    INTERACTIVE_SCREEN_VSYNC_TEST = 3,
} interactive_screen_t;

static void _print_synopsis()
{
    printfln_err("D3D9 monitor check tool interactive mode");
    printfln_err("");
    printfln_err("Usage:");
    printfln_err("  d3d9-monitor-check interactive <width> <height> <refresh-rate> [--windowed] [--vsync-off]");
    printfln_err("");
    printfln_err("    width: Width of the rendering resolution to run the test at");
    printfln_err("    height: Height of the rendering resolution to run the test at");
    printfln_err("    refresh-rate: Target refresh rate to run the test at");
    printfln_err("    windowed: Optional. Run the test in windowed mode (not recommended)");
    printfln_err("    vsync-off: Optional. Run the test with vsync off (not recommended)");
}

static bool _interactive(
    uint32_t width,
    uint32_t height,
    uint32_t refresh_rate,
    bool windowed,
    bool vsync)
{
    gfx_t *gfx;
    input_t *input;
    bool loop_running;
    interactive_screen_t current_screen;
    menu_item_t selected_menu_item;

    menu_t *menu;
    refresh_rate_test_t *refresh_rate_test;
    response_time_test_t *response_time_test;
    vsync_test_t *vsync_test;

    input_init(&input);

    if (!gfx_init(width, height, refresh_rate, windowed, vsync, &gfx)) {
        input_fini(input);
        return false;
    }

    if (!menu_init(gfx, &menu)) {
        gfx_fini(gfx);
        input_fini(input);
        return false;
    }

    loop_running = true;
    current_screen = INTERACTIVE_SCREEN_MENU;

    while (loop_running) {
        input_update(input);

        switch (current_screen) {
            case INTERACTIVE_SCREEN_MENU:
                if (input_key_esc_pushed(input)) {
                    loop_running = false;
                    break;
                } else if (input_key_up_pushed(input)) {
                    menu_select_cursor_move_up(menu);
                } else if (input_key_down_pushed(input)) {
                    menu_select_cursor_move_down(menu);
                } else if (input_key_enter_pushed(input)) {
                    selected_menu_item = menu_item_selected_get(menu);

                    switch (selected_menu_item) {                           
                        case MENU_ITEM_REFRESH_RATE_TEST:
                            if (!refresh_rate_test_init(gfx, 600, 1200 ,&refresh_rate_test)) {
                                loop_running = false;
                                break;
                            }

                            current_screen = INTERACTIVE_SCREEN_REFRESH_RATE_TEST;
                            break;
                            
                        case MENU_ITEM_RESPONSE_TIME_TEST:
                            if (!response_time_test_init(gfx, &response_time_test)) {
                                loop_running = false;
                                break;
                            }

                            current_screen = INTERACTIVE_SCREEN_RESPONSE_TIME_TEST;
                            break;

                        case MENU_ITEM_VSYNC_TEST:
                            if (!vsync_test_init(gfx, &vsync_test)) {
                                loop_running = false;
                                break;
                            }

                            current_screen = INTERACTIVE_SCREEN_VSYNC_TEST;
                            break;

                        case MENU_ITEM_EXIT:
                            loop_running = false;
                            break;

                        default:
                            assert(0);
                            break;
                    }
                }

                if (!menu_frame_update(menu)) {
                    loop_running = false;
                    break;
                }

                break;

            case INTERACTIVE_SCREEN_REFRESH_RATE_TEST:
                if (    input_key_esc_pushed(input) || 
                        !refresh_rate_test_frame_update(refresh_rate_test)) {
                    current_screen = INTERACTIVE_SCREEN_MENU;
                    refresh_rate_test_fini(refresh_rate_test);
                }
         
                break;

            case INTERACTIVE_SCREEN_RESPONSE_TIME_TEST:
                if (    input_key_esc_pushed(input) || 
                        !response_time_test_frame_update(response_time_test)) {
                    current_screen = INTERACTIVE_SCREEN_MENU;
                    response_time_test_fini(response_time_test);
                }

                break;

            case INTERACTIVE_SCREEN_VSYNC_TEST:
                if (    input_key_esc_pushed(input) || 
                        !vsync_test_frame_update(vsync_test)) {
                    current_screen = INTERACTIVE_SCREEN_MENU;
                    vsync_test_fini(vsync_test);
                }

                break;
        }
    }

    menu_fini(menu);

    gfx_fini(gfx);
    input_fini(input);

    return true;
}

bool interactive_main(int argc, char **argv)
{
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
    bool windowed;
    bool vsync;
    
    assert(argv);

    if (argc < 3) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }
    
    width = atoi(argv[0]);
    height = atoi(argv[1]);
    refresh_rate = atoi(argv[2]);
    windowed = false;
    vsync = true;

    for (int i = 3; i < argc; i++) {
        if (!strcmp(argv[i], "--windowed")) {
            windowed = true;
        } else if (!strcmp(argv[i], "--vsync-off")) {
            vsync = false;
        }
    }
    
    return _interactive(width, height, refresh_rate, windowed, vsync);
}