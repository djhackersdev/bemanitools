#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"
#include "core/thread-crt.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface-io/ddr.h"

#include "module/io-ext.h"

static void _ddriotest_io_ddr_init(module_io_t **module)
{
    bt_io_ddr_api_t api;

    module_io_ext_load_and_init(
        "ddrio.dll", "bt_module_io_ddr_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_ddr_api_set(&api);
}

int main(int argc, char **argv)
{
    enum core_log_bt_log_level log_level;
    module_io_t *module_io_ddr;

    log_level = CORE_LOG_BT_LOG_LEVEL_FATAL;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            log_level = CORE_LOG_BT_LOG_LEVEL_WARNING;
        } else if (!strcmp(argv[i], "-vv")) {
            log_level = CORE_LOG_BT_LOG_LEVEL_INFO;
        } else if (!strcmp(argv[i], "-vvv")) {
            log_level = CORE_LOG_BT_LOG_LEVEL_MISC;
        }
    }

    core_log_bt_core_api_set();
    core_thread_crt_core_api_set();

    core_log_bt_ext_init_with_stderr();
    core_log_bt_level_set(log_level);

    _ddriotest_io_ddr_init(&module_io_ddr);

    if (!bt_io_ddr_init()) {
        fprintf(stderr, "Initializing ddrio failed\n");

        bt_io_ddr_api_clear();
        module_io_free(&module_io_ddr);

        return -1;
    }

    fprintf(
        stderr,
        ">>> Initializing ddrio successful, press enter to continue <<<\n");

    if (getchar() != '\n') {
        bt_io_ddr_fini();

        bt_io_ddr_api_clear();
        module_io_free(&module_io_ddr);

        return 0;
    }

    // inputs
    uint32_t pad = 0;

    // outputs
    uint32_t extio_lights = 0;
    uint32_t p3io_lights = 0;

    bool loop = true;
    uint8_t cnt = 0;

    while (loop) {
        bt_io_ddr_extio_lights_set(extio_lights);
        bt_io_ddr_p3io_lights_set(p3io_lights);

        pad = bt_io_ddr_pad_read();

        system("cls");

        printf("Counter: %d\n", cnt);
        printf("Player 1       Player 2\n");
        printf("\n");
        printf("Pad\n");

        printf(
            "Up:    %d       Up:    %d\n",
            (pad & (1 << BT_IO_DDR_P1_UP)) > 0,
            (pad & (1 << BT_IO_DDR_P2_UP)) > 0);
        printf(
            "Down:  %d       Down:  %d\n",
            (pad & (1 << BT_IO_DDR_P1_DOWN)) > 0,
            (pad & (1 << BT_IO_DDR_P2_DOWN)) > 0);
        printf(
            "Left:  %d       Left:  %d\n",
            (pad & (1 << BT_IO_DDR_P1_LEFT)) > 0,
            (pad & (1 << BT_IO_DDR_P2_LEFT)) > 0);
        printf(
            "Right: %d       Right: %d\n",
            (pad & (1 << BT_IO_DDR_P1_RIGHT)) > 0,
            (pad & (1 << BT_IO_DDR_P2_RIGHT)) > 0);
        printf("\n");
        printf("Menu\n");
        printf(
            "Start: %d       Start: %d\n",
            (pad & (1 << BT_IO_DDR_P1_START)) > 0,
            (pad & (1 << BT_IO_DDR_P2_START)) > 0);
        printf(
            "Up:    %d       Up:    %d\n",
            (pad & (1 << BT_IO_DDR_P1_MENU_UP)) > 0,
            (pad & (1 << BT_IO_DDR_P2_MENU_UP)) > 0);
        printf(
            "Down:  %d       Down:  %d\n",
            (pad & (1 << BT_IO_DDR_P1_MENU_DOWN)) > 0,
            (pad & (1 << BT_IO_DDR_P2_MENU_DOWN)) > 0);
        printf(
            "Left:  %d       Left:  %d\n",
            (pad & (1 << BT_IO_DDR_P1_MENU_LEFT)) > 0,
            (pad & (1 << BT_IO_DDR_P2_MENU_LEFT)) > 0);
        printf(
            "Right: %d       Right: %d\n",
            (pad & (1 << BT_IO_DDR_P1_MENU_RIGHT)) > 0,
            (pad & (1 << BT_IO_DDR_P2_MENU_RIGHT)) > 0);
        printf("\n");
        printf("Operator\n");
        printf(
            "Test: %d   Service: %d   Coin: %d\n",
            (pad & (1 << BT_IO_DDR_TEST)) > 0,
            (pad & (1 << BT_IO_DDR_SERVICE)) > 0,
            (pad & (1 << BT_IO_DDR_COIN)) > 0);

        if ((pad & (1 << BT_IO_DDR_P1_UP)) > 0) {
            extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_P1_UP);
        } else {
            extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_P1_UP);
        }

        if ((pad & (1 << BT_IO_DDR_P1_DOWN)) > 0) {
            extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_P1_DOWN);
        } else {
            extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_P1_DOWN);
        }

        if ((pad & (1 << BT_IO_DDR_P1_LEFT)) > 0) {
            extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_P1_LEFT);
        } else {
            extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_P1_LEFT);
        }

        if ((pad & (1 << BT_IO_DDR_P1_RIGHT)) > 0) {
            extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_P1_RIGHT);
        } else {
            extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_P1_RIGHT);
        }

        if ((pad & (1 << BT_IO_DDR_P2_UP)) > 0) {
            extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_P2_UP);
        } else {
            extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_P2_UP);
        }

        if ((pad & (1 << BT_IO_DDR_P2_DOWN)) > 0) {
            extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_P2_DOWN);
        } else {
            extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_P2_DOWN);
        }

        if ((pad & (1 << BT_IO_DDR_P2_LEFT)) > 0) {
            extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_P2_LEFT);
        } else {
            extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_P2_LEFT);
        }

        if ((pad & (1 << BT_IO_DDR_P2_RIGHT)) > 0) {
            extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_P2_RIGHT);
        } else {
            extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_P2_RIGHT);
        }

        if ((pad & (1 << BT_IO_DDR_P1_START)) > 0 ||
            (pad & (1 << BT_IO_DDR_P1_MENU_UP)) > 0 ||
            (pad & (1 << BT_IO_DDR_P1_MENU_DOWN)) > 0 ||
            (pad & (1 << BT_IO_DDR_P1_MENU_LEFT)) > 0 ||
            (pad & (1 << BT_IO_DDR_P1_MENU_RIGHT)) > 0) {
            p3io_lights |= (1 << BT_IO_DDR_P3IO_LIGHT_P1_MENU);
        } else {
            p3io_lights &= ~(1 << BT_IO_DDR_P3IO_LIGHT_P1_MENU);
        }

        if ((pad & (1 << BT_IO_DDR_P2_START)) > 0 ||
            (pad & (1 << BT_IO_DDR_P2_MENU_UP)) > 0 ||
            (pad & (1 << BT_IO_DDR_P2_MENU_DOWN)) > 0 ||
            (pad & (1 << BT_IO_DDR_P2_MENU_LEFT)) > 0 ||
            (pad & (1 << BT_IO_DDR_P2_MENU_RIGHT)) > 0) {
            p3io_lights |= (1 << BT_IO_DDR_P3IO_LIGHT_P2_MENU);
        } else {
            p3io_lights &= ~(1 << BT_IO_DDR_P3IO_LIGHT_P2_MENU);
        }

        /* avoid CPU banging */
        Sleep(1);
        ++cnt;

        /* process menu */
        if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
            system("cls");
            Sleep(5);
            printf(
                "Menu options:\n"
                "  0: Exit menu and continue loop\n"
                "  1: Exit\n"
                "  2: Set neon state\n"
                "  3: Top lamp state\n"
                "  4: Set all lights on\n"
                "  5: Set all lights off\n"
                "Waiting for input: ");

            char c = getchar();

            switch (c) {
                case '1': {
                    loop = false;
                    break;
                }

                case '2': {
                    int state;
                    int n;

                    printf("Enter neon state (0/1): ");

                    n = scanf("%d", &state);

                    if (n > 0) {
                        extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_NEONS);
                    } else {
                        extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_NEONS);
                    }

                    break;
                }

                case '3': {
                    char buf[4];
                    int n;

                    printf("Enter top lamp state, chain of 0/1s: ");

                    n = scanf("%8s", buf);

                    if (n > 0) {
                        if (buf[0] == '1') {
                            p3io_lights |=
                                (1 << BT_IO_DDR_P3IO_LIGHT_P1_UPPER_LAMP);
                        } else {
                            p3io_lights &=
                                ~(1 << BT_IO_DDR_P3IO_LIGHT_P1_UPPER_LAMP);
                        }

                        if (buf[1] == '1') {
                            p3io_lights |=
                                (1 << BT_IO_DDR_P3IO_LIGHT_P1_LOWER_LAMP);
                        } else {
                            p3io_lights &=
                                ~(1 << BT_IO_DDR_P3IO_LIGHT_P1_LOWER_LAMP);
                        }

                        if (buf[2] == '1') {
                            p3io_lights |=
                                (1 << BT_IO_DDR_P3IO_LIGHT_P2_UPPER_LAMP);
                        } else {
                            p3io_lights &=
                                ~(1 << BT_IO_DDR_P3IO_LIGHT_P2_UPPER_LAMP);
                        }

                        if (buf[3] == '1') {
                            p3io_lights |=
                                (1 << BT_IO_DDR_P3IO_LIGHT_P2_LOWER_LAMP);
                        } else {
                            p3io_lights &=
                                ~(1 << BT_IO_DDR_P3IO_LIGHT_P2_LOWER_LAMP);
                        }
                    }

                    break;
                }

                case '4': {
                    extio_lights |= (1 << BT_IO_DDR_EXTIO_LIGHT_NEONS);

                    p3io_lights |= (1 << BT_IO_DDR_P3IO_LIGHT_P1_MENU);
                    p3io_lights |= (1 << BT_IO_DDR_P3IO_LIGHT_P2_MENU);
                    p3io_lights |= (1 << BT_IO_DDR_P3IO_LIGHT_P1_UPPER_LAMP);
                    p3io_lights |= (1 << BT_IO_DDR_P3IO_LIGHT_P1_LOWER_LAMP);
                    p3io_lights |= (1 << BT_IO_DDR_P3IO_LIGHT_P2_UPPER_LAMP);
                    p3io_lights |= (1 << BT_IO_DDR_P3IO_LIGHT_P2_LOWER_LAMP);

                    break;
                }

                case '5': {
                    extio_lights &= ~(1 << BT_IO_DDR_EXTIO_LIGHT_NEONS);

                    p3io_lights &= ~(1 << BT_IO_DDR_P3IO_LIGHT_P1_MENU);
                    p3io_lights &= ~(1 << BT_IO_DDR_P3IO_LIGHT_P2_MENU);
                    p3io_lights &= ~(1 << BT_IO_DDR_P3IO_LIGHT_P1_UPPER_LAMP);
                    p3io_lights &= ~(1 << BT_IO_DDR_P3IO_LIGHT_P1_LOWER_LAMP);
                    p3io_lights &= ~(1 << BT_IO_DDR_P3IO_LIGHT_P2_UPPER_LAMP);
                    p3io_lights &= ~(1 << BT_IO_DDR_P3IO_LIGHT_P2_LOWER_LAMP);

                    break;
                }

                case '0':
                default:
                    break;
            }
        }
    }

    system("cls");

    bt_io_ddr_fini();

    bt_io_ddr_api_clear();
    module_io_free(&module_io_ddr);

    return 0;
}