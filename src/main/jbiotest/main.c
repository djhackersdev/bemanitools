#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "bemanitools/jbio.h"

#include "util/log.h"
#include "util/thread.h"

typedef struct {
    uint8_t r, g, b;
} rgb_t;

enum jbio_light_mode {
    LIGHTS_OFF,
    LIGHTS_ON,
    LIGHTS_INPUT
};

#define IS_BIT_SET(var, bit) ((((var) >> (bit)) & 1) > 0)

/**
 * Tool to test your implementations of jbio.
 */
int main(int argc, char **argv)
{
    log_to_writer(log_writer_stdout, NULL);

    jb_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!jb_io_init(crt_thread_create, crt_thread_join, crt_thread_destroy)) {
        printf("Initializing jbio failed\n");
        return -1;
    }

    printf(">>> Initializing jbio successful, press enter to continue <<<\n");

    if (getchar() != '\n') {
        return 0;
    }

    /* inputs */
    uint8_t input_sys = 0;
    uint16_t input_panel = 0;

    /* outputs */
    rgb_t lights[6] = {0};

    bool loop = true;
    uint8_t cnt = 0;
    enum jbio_light_mode light_mode = LIGHTS_OFF;

    while (loop) {
        if (!jb_io_read_inputs()) {
            printf("ERROR: Input read fail\n");
            return -2;
        }

        /* get inputs */
        input_sys = jb_io_get_sys_inputs();
        input_panel = jb_io_get_panel_inputs();

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
            "          | %d | %d | %d | %d |\n"
            "          |---|---|---|---|\n"
            "          | %d | %d | %d | %d |\n"
            "          |---|---|---|---|\n"
            "          | %d | %d | %d | %d |\n"
            "          |---|---|---|---|\n"
            "          | %d | %d | %d | %d |\n"
            "          `---`---`---`---`\n",
            cnt,

            lights[JB_IO_RGB_LED_TOP].r,
            lights[JB_IO_RGB_LED_TOP].g,
            lights[JB_IO_RGB_LED_TOP].b,
            lights[JB_IO_RGB_LED_FRONT].r,
            lights[JB_IO_RGB_LED_FRONT].g,
            lights[JB_IO_RGB_LED_FRONT].b,
            lights[JB_IO_RGB_LED_LEFT].r,
            lights[JB_IO_RGB_LED_RIGHT].r,
            lights[JB_IO_RGB_LED_LEFT].g,
            lights[JB_IO_RGB_LED_RIGHT].g,
            lights[JB_IO_RGB_LED_LEFT].b,
            lights[JB_IO_RGB_LED_TITLE].r,
            lights[JB_IO_RGB_LED_TITLE].g,
            lights[JB_IO_RGB_LED_TITLE].b,
            lights[JB_IO_RGB_LED_RIGHT].b,
            lights[JB_IO_RGB_LED_WOOFER].r,
            lights[JB_IO_RGB_LED_WOOFER].g,
            lights[JB_IO_RGB_LED_WOOFER].b,

            IS_BIT_SET(input_sys, JB_IO_SYS_TEST),
            IS_BIT_SET(input_sys, JB_IO_SYS_SERVICE),
            IS_BIT_SET(input_sys, JB_IO_SYS_COIN),

            IS_BIT_SET(input_panel, JB_IO_PANEL_01),
            IS_BIT_SET(input_panel, JB_IO_PANEL_02),
            IS_BIT_SET(input_panel, JB_IO_PANEL_03),
            IS_BIT_SET(input_panel, JB_IO_PANEL_04),
            IS_BIT_SET(input_panel, JB_IO_PANEL_05),
            IS_BIT_SET(input_panel, JB_IO_PANEL_06),
            IS_BIT_SET(input_panel, JB_IO_PANEL_07),
            IS_BIT_SET(input_panel, JB_IO_PANEL_08),
            IS_BIT_SET(input_panel, JB_IO_PANEL_09),
            IS_BIT_SET(input_panel, JB_IO_PANEL_10),
            IS_BIT_SET(input_panel, JB_IO_PANEL_11),
            IS_BIT_SET(input_panel, JB_IO_PANEL_12),
            IS_BIT_SET(input_panel, JB_IO_PANEL_13),
            IS_BIT_SET(input_panel, JB_IO_PANEL_14),
            IS_BIT_SET(input_panel, JB_IO_PANEL_15),
            IS_BIT_SET(input_panel, JB_IO_PANEL_16));

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
            jb_io_set_rgb_led(i, lights[i].r, lights[i].g, lights[i].b);
        }

        if (!jb_io_write_outputs()) {
            printf("ERROR: Writing outputs failed\n");
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
                "Waiting for input: ");
            char c = getchar();

            switch (c) {
                case '1': {
                    /* one last update to turn off the lights */
                    for (int i = 0; i < 6; i++) {
                        jb_io_set_rgb_led(i, 0, 0, 0);
                    }

                    if (!jb_io_write_outputs()) {
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

                case '0':
                default:
                    break;
            }
        }
    }

    system("cls");
    jb_io_fini();

    return 0;
}
