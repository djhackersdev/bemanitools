#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"

#include "iface-core/log.h"
#include "iface-io/jb.h"

#include "module/io-ext.h"

typedef struct {
    uint8_t r, g, b;
} rgb_t;

enum jbio_light_mode { LIGHTS_OFF, LIGHTS_ON, LIGHTS_INPUT };

#define IS_BIT_SET(var, bit) ((((var) >> (bit)) & 1) > 0)

static void _jbiotest_io_jb_init(module_io_t **module)
{
    bt_io_jb_api_t api;

    module_io_ext_load_and_init("jbio.dll", "bt_module_io_jb_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_jb_api_set(&api);
}

/**
 * Tool to test your implementations of jbio.
 */
int main(int argc, char **argv)
{
    module_io_t *module_io_jb;

    core_log_bt_core_api_set();

    core_log_bt_ext_init_with_stderr();
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);

    _jbiotest_io_jb_init(&module_io_jb);

    if (!bt_io_jb_init()) {
        printf("Initializing jbio failed\n");

        bt_io_jb_api_clear();
        module_io_free(&module_io_jb);

        return -1;
    }

    printf(">>> Initializing jbio successful, press enter to continue <<<\n");

    if (getchar() != '\n') {
        bt_io_jb_fini();

        bt_io_jb_api_clear();
        module_io_free(&module_io_jb);

        return 0;
    }

    /* inputs */
    uint8_t input_sys = 0;
    uint16_t input_panel = 0;

    /* outputs */
    rgb_t lights[6] = {0};

    bool loop = true;
    uint16_t cnt = 0;
    enum jbio_light_mode light_mode = LIGHTS_OFF;
    bt_io_jb_panel_mode_t panel_mode = BT_IO_JB_PANEL_MODE_ALL;

    bool panel_corners_animate = false;
    char *all_text;
    char top_left, top_right, bottom_left, bottom_right;

    while (loop) {
        if (!bt_io_jb_inputs_read()) {
            printf("ERROR: Input read fail\n");

            bt_io_jb_fini();

            bt_io_jb_api_clear();
            module_io_free(&module_io_jb);

            return -2;
        }

        if (panel_corners_animate && (cnt % 50) == 0) {
            panel_mode++;
            panel_mode %= 5;

            // skip the all state
            if (panel_mode == BT_IO_JB_PANEL_MODE_ALL) {
                panel_mode++;
            }

            bt_io_jb_panel_mode_set(panel_mode);
        }

        all_text = "   ";
        top_left = top_right = bottom_left = bottom_right = ' ';
        switch (panel_mode) {
            case BT_IO_JB_PANEL_MODE_ALL:
                all_text = "ALL";
                top_left = top_right = bottom_left = bottom_right = '*';
                break;
            case BT_IO_JB_PANEL_MODE_TOP_LEFT:
                top_left = '*';
                break;
            case BT_IO_JB_PANEL_MODE_TOP_RIGHT:
                top_right = '*';
                break;
            case BT_IO_JB_PANEL_MODE_BOTTOM_LEFT:
                bottom_left = '*';
                break;
            case BT_IO_JB_PANEL_MODE_BOTTOM_RIGHT:
                bottom_right = '*';
                break;
        }

        /* get inputs */
        input_sys = bt_io_jb_sys_inputs_get();
        input_panel = bt_io_jb_panel_inputs_get();

        system("cls");
        printf(
            "Press escape to open menu\n"
            "%d\n"
            ".___________________________________.\n"
            "|'.                               .'|\n"
            "|  '.            Top            .'  |\n"
            "|    '.   R:%03d G:%03d B:%03d   .'    |\n"
            "|      '.___________________.'      |\n"
            "|       |       Front       |       |\n"
            "| Left  | R:%03d G:%03d B:%03d | Right |\n"
            "| R:%03d |                   | R:%03d |\n"
            "| G:%03d |       Title       | G:%03d |\n"
            "| B:%03d | R:%03d G:%03d B:%03d | B:%03d |\n"
            "|       |                   |       |\n"
            "|       |       Woofer      |       |\n"
            "|       | R:%03d G:%03d B:%03d |       |\n"
            "|_______|___________________|_______|\n"
            "\n"
            " Test: %d      Service: %d     Coin: %d\n"
            "          .___.___.___.___.\n"
            "          | %d | %d | %d | %d |    Corner\n"
            "          |---|---|---|---|    .___.\n"
            "          | %d | %d | %d | %d |    |%c %c|\n"
            "          |---|---|---|---|    |%s|\n"
            "          | %d | %d | %d | %d |    |%c %c|\n"
            "          |---|---|---|---|    `---`\n"
            "          | %d | %d | %d | %d |\n"
            "          `---`---`---`---`\n",
            cnt,

            lights[BT_IO_JB_RGB_LED_TOP].r,
            lights[BT_IO_JB_RGB_LED_TOP].g,
            lights[BT_IO_JB_RGB_LED_TOP].b,
            lights[BT_IO_JB_RGB_LED_FRONT].r,
            lights[BT_IO_JB_RGB_LED_FRONT].g,
            lights[BT_IO_JB_RGB_LED_FRONT].b,
            lights[BT_IO_JB_RGB_LED_LEFT].r,
            lights[BT_IO_JB_RGB_LED_RIGHT].r,
            lights[BT_IO_JB_RGB_LED_LEFT].g,
            lights[BT_IO_JB_RGB_LED_RIGHT].g,
            lights[BT_IO_JB_RGB_LED_LEFT].b,
            lights[BT_IO_JB_RGB_LED_TITLE].r,
            lights[BT_IO_JB_RGB_LED_TITLE].g,
            lights[BT_IO_JB_RGB_LED_TITLE].b,
            lights[BT_IO_JB_RGB_LED_RIGHT].b,
            lights[BT_IO_JB_RGB_LED_WOOFER].r,
            lights[BT_IO_JB_RGB_LED_WOOFER].g,
            lights[BT_IO_JB_RGB_LED_WOOFER].b,

            IS_BIT_SET(input_sys, BT_IO_JB_SYS_TEST),
            IS_BIT_SET(input_sys, BT_IO_JB_SYS_SERVICE),
            IS_BIT_SET(input_sys, BT_IO_JB_SYS_COIN),

            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_01),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_02),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_03),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_04),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_05),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_06),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_07),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_08),

            top_left,
            top_right,
            all_text,

            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_09),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_10),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_11),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_12),

            bottom_left,
            bottom_right,

            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_13),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_14),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_15),
            IS_BIT_SET(input_panel, BT_IO_JB_PANEL_16));

        /* set outputs */
        switch (light_mode) {
            case LIGHTS_OFF:
                memset(lights, 0x00, sizeof(lights));
                break;

            case LIGHTS_ON:
                memset(lights, 0xFF, sizeof(lights));
                if (input_panel) {
                    /* disable all on when a single button is pressed */
                    light_mode = LIGHTS_OFF;
                }
                break;

            case LIGHTS_INPUT: {
                uint32_t all_inputs = input_panel | (input_sys << 16);
                for (int i = 0; i < 6; i++) {
                    lights[i].r = (all_inputs & 1) ? 0xFF : 0x00;
                    lights[i].g = (all_inputs & 2) ? 0xFF : 0x00;
                    lights[i].b = (all_inputs & 4) ? 0xFF : 0x00;
                    all_inputs >>= 3;
                }
                break;
            }
        }

        for (int i = 0; i < 6; i++) {
            bt_io_jb_rgb_led_set(i, lights[i].r, lights[i].g, lights[i].b);
        }

        if (!bt_io_jb_lights_write()) {
            printf("ERROR: Writing outputs failed\n");

            bt_io_jb_fini();

            bt_io_jb_api_clear();
            module_io_free(&module_io_jb);

            return -4;
        }

        /* avoid CPU banging */
        Sleep(5);
        ++cnt;

        /* process menu */
        if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
            system("cls");
            Sleep(5);
            printf(
                "Menu options:\n"
                "  0: Exit menu and continue loop\n"
                "  1: Exit\n"
                "  2: Set all lights on (cleared by pressing any panel)\n"
                "  3: Tie each R/G/B to inputs (panels + test + service)\n"
                "  4: Set all lights off\n"
                "  5: Toggle switch corner test mode\n"
                "Waiting for input: ");
            char c = getchar();

            switch (c) {
                case '1': {
                    /* one last update to turn off the lights */
                    for (int i = 0; i < 6; i++) {
                        bt_io_jb_rgb_led_set(i, 0, 0, 0);
                    }

                    if (!bt_io_jb_lights_write()) {
                        printf("ERROR: Writing outputs failed\n");
                        return -4;
                    }

                    loop = false;
                    break;
                }

                case '2': {
                    light_mode = LIGHTS_ON;
                    break;
                }

                case '3': {
                    light_mode = LIGHTS_INPUT;
                    break;
                }

                case '4': {
                    light_mode = LIGHTS_OFF;
                    break;
                }

                case '5': {
                    panel_corners_animate = !panel_corners_animate;
                    if (!panel_corners_animate) {
                        panel_mode = BT_IO_JB_PANEL_MODE_ALL;
                        bt_io_jb_panel_mode_set(panel_mode);
                    }
                    break;
                }

                case '0':
                default:
                    break;
            }
        }
    }

    system("cls");

    bt_io_jb_fini();

    bt_io_jb_api_clear();
    module_io_free(&module_io_jb);

    return 0;
}
