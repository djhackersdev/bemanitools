#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"

#include "iface-core/log.h"
#include "iface-io/iidx.h"

#include "module/io-ext.h"

static uint8_t _fix_top_lamps_order(uint8_t top_lamps)
{
    uint8_t out;

    out = 0;

    out |= ((1 << 0) & top_lamps) > 0 ? (1 << BT_IO_IIDX_TOP_LAMP_LEFT_RED) : 0;
    out |=
        ((1 << 1) & top_lamps) > 0 ? (1 << BT_IO_IIDX_TOP_LAMP_LEFT_YELLOW) : 0;
    out |=
        ((1 << 2) & top_lamps) > 0 ? (1 << BT_IO_IIDX_TOP_LAMP_LEFT_GREEN) : 0;
    out |=
        ((1 << 3) & top_lamps) > 0 ? (1 << BT_IO_IIDX_TOP_LAMP_LEFT_BLUE) : 0;

    out |=
        ((1 << 4) & top_lamps) > 0 ? (1 << BT_IO_IIDX_TOP_LAMP_RIGHT_BLUE) : 0;
    out |=
        ((1 << 5) & top_lamps) > 0 ? (1 << BT_IO_IIDX_TOP_LAMP_RIGHT_GREEN) : 0;
    out |= ((1 << 6) & top_lamps) > 0 ?
        (1 << BT_IO_IIDX_TOP_LAMP_RIGHT_YELLOW) :
        0;
    out |=
        ((1 << 7) & top_lamps) > 0 ? (1 << BT_IO_IIDX_TOP_LAMP_RIGHT_RED) : 0;

    return out;
}

static void _all_lights_off_shutdown()
{
    bt_io_iidx_ep1_deck_lights_set(0);
    bt_io_iidx_ep1_panel_lights_set(0);
    bt_io_iidx_ep1_top_lamps_set(0);
    bt_io_iidx_ep1_top_neons_set(false);

    // Depending on the IO, pushing the state to the actual outputs, e.g. lights
    // on/off can be a bit finicky. Do a few times to "enforce" the state
    for (uint8_t i = 0; i < 3; i++) {
        bt_io_iidx_ep1_send();
        // Required to handle iidxio-ezusb specific quirks with flushing 16seg
        // text
        bt_io_iidx_ep2_recv();
        bt_io_iidx_ep3_16seg_send("         ");

        Sleep(10);
    }
}

static void _iidxiotest_io_iidx_init(module_io_t **module)
{
    bt_io_iidx_api_t api;

    module_io_ext_load_and_init(
        "iidxio.dll", "bt_module_io_iidx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_iidx_api_set(&api);
}

/**
 * Tool to test your implementations of iidxio.
 */
int main(int argc, char **argv)
{
    module_io_t *module_io_iidx;

    core_log_bt_core_api_set();

    core_log_bt_ext_init_with_stderr();
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);

    _iidxiotest_io_iidx_init(&module_io_iidx);

    if (!bt_io_iidx_init()) {
        printf("Initializing iidxio failed\n");

        bt_io_iidx_api_clear();
        module_io_free(&module_io_iidx);

        return -1;
    }

    printf(">>> Initializing iidxio successful, press enter to continue <<<\n");

    if (getchar() != '\n') {
        bt_io_iidx_fini();

        bt_io_iidx_api_clear();
        module_io_free(&module_io_iidx);

        return 0;
    }

    /* inputs */
    uint8_t input_sys = 0;
    uint8_t input_panel = 0;
    uint16_t input_keys = 0;
    uint8_t turn_table[2] = {0, 0};
    uint8_t slider[5] = {0, 0, 0, 0, 0};

    /* outputs */
    uint16_t deck_lights = 0;
    uint8_t panel_lights = 0;
    uint8_t top_lamps = 0;
    bool top_neons = false;
    char text_16seg[9] = "         ";

    bool loop = true;
    uint8_t cnt = 0;
    bool all_on = false;
    while (loop) {
        if (!bt_io_iidx_ep2_recv()) {
            printf("ERROR: Receive ep2 failed\n");

            bt_io_iidx_fini();

            bt_io_iidx_api_clear();
            module_io_free(&module_io_iidx);

            return -2;
        }

        /* get inputs */
        input_sys = bt_io_iidx_ep2_sys_get();
        input_panel = bt_io_iidx_ep2_panel_get();
        input_keys = bt_io_iidx_ep2_keys_get();
        turn_table[0] = bt_io_iidx_ep2_turntable_get(0);
        turn_table[1] = bt_io_iidx_ep2_turntable_get(1);
        for (uint8_t i = 0; i < 5; ++i) {
            slider[i] = bt_io_iidx_ep2_slider_get(i);
        }

        system("cls");
        printf(
            "Press escape to open menu\n"
            "%d\n"
            "|---------------------------------------|\n"
            "| R  Y  G  B      Neons      B  G  Y  R |\n"
            "| %d  %d  %d  %d        %d        %d  %d  %d  %d |\n"
            "|---------------------------------------|\n"
            "|   NOW PLAYING: %c%c%c%c%c%c%c%c%c              |\n"
            "|---------------------------------------|\n"
            "| Effect %d  S1  S2  S3  S4  S5    Test %d|\n"
            "|StartP1 %d  %02d  %02d  %02d  %02d  %02d StartP2 %d|\n"
            "|   VEFX %d                     Service %d|\n"
            "_________________________________________\n"
            "|   __                             __   |\n"
            "|  /   \\            _             /   \\ |\n"
            "| | %03d|    %d %d %d  |%d|  %d %d %d    | %03d| |\n"
            "|  \\___/   %d %d %d %d |_| %d %d %d %d    \\___/ |\n"
            "|                                       |\n"
            "|---------------------------------------|\n"
            "|---------------------------------------|\n",
            cnt,

            (top_lamps & (1 << 0)) > 0,
            (top_lamps & (1 << 1)) > 0,
            (top_lamps & (1 << 2)) > 0,
            (top_lamps & (1 << 3)) > 0,
            top_neons,
            (top_lamps & (1 << 4)) > 0,
            (top_lamps & (1 << 5)) > 0,
            (top_lamps & (1 << 6)) > 0,
            (top_lamps & (1 << 7)) > 0,

            text_16seg[0],
            text_16seg[1],
            text_16seg[2],
            text_16seg[3],
            text_16seg[4],
            text_16seg[5],
            text_16seg[6],
            text_16seg[7],
            text_16seg[8],
            (input_panel >> BT_IO_IIDX_PANEL_EFFECT) & 1,
            (input_sys >> BT_IO_IIDX_SYS_TEST) & 1,

            (input_panel >> BT_IO_IIDX_PANEL_P1_START) & 1,
            slider[0],
            slider[1],
            slider[2],
            slider[3],
            slider[4],
            (input_panel >> BT_IO_IIDX_PANEL_P2_START) & 1,

            (input_panel >> BT_IO_IIDX_PANEL_VEFX) & 1,
            (input_sys >> BT_IO_IIDX_SYS_SERVICE) & 1,

            turn_table[0],
            (input_keys >> BT_IO_IIDX_KEY_P1_2) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P1_4) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P1_6) & 1,
            (input_sys >> BT_IO_IIDX_SYS_COIN),
            (input_keys >> BT_IO_IIDX_KEY_P2_2) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P2_4) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P2_6) & 1,
            turn_table[1],
            (input_keys >> BT_IO_IIDX_KEY_P1_1) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P1_3) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P1_5) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P1_7) & 1,

            (input_keys >> BT_IO_IIDX_KEY_P2_1) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P2_3) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P2_5) & 1,
            (input_keys >> BT_IO_IIDX_KEY_P2_7) & 1);

        /* set outputs */
        if (all_on) {
            deck_lights = 0x3FFF;
            panel_lights = 0xF;
            top_lamps = 0xFF;
            top_neons = true;
            memset(text_16seg, '*', 9);
        }

        bt_io_iidx_ep1_deck_lights_set(deck_lights);
        bt_io_iidx_ep1_panel_lights_set(panel_lights);
        bt_io_iidx_ep1_top_lamps_set(_fix_top_lamps_order(top_lamps));
        bt_io_iidx_ep1_top_neons_set(top_neons);

        /* light up keys when pressed */
        if (!all_on) {
            deck_lights = input_keys;
            panel_lights = input_panel;
        } else {
            /* disable all on when a single button is pressed */
            if (input_keys || input_panel) {
                all_on = false;
                deck_lights = 0;
                panel_lights = 0;
                top_lamps = 0;
                top_neons = false;
                memset(text_16seg, ' ', 9);
            }
        }

        if (!bt_io_iidx_ep1_send()) {
            printf("ERROR: Sending ep1 failed\n");

            bt_io_iidx_fini();

            bt_io_iidx_api_clear();
            module_io_free(&module_io_iidx);

            return -4;
        }

        if (!bt_io_iidx_ep3_16seg_send(text_16seg)) {
            printf("ERROR: Sending ep3 failed\n");

            bt_io_iidx_fini();

            bt_io_iidx_api_clear();
            module_io_free(&module_io_iidx);

            return -3;
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
                "  2: Enter text for 16seg display\n"
                "  3: Set neon state\n"
                "  4: Set top lamp state\n"
                "  5: Set all outputs on (cleared by pressing any IIDX "
                "button)\n"
                "  6: Clear all outputs\n"
                "Waiting for input: ");
            char c = getchar();

            switch (c) {
                case '1': {
                    loop = false;
                    break;
                }

                case '2': {
                    char buf[10];
                    printf("Enter 16seg text (max 9 chars): ");
                    int n = scanf("%9s", buf);

                    if (n > 0) {
                        n = strlen(buf);

                        if (n > 9) {
                            n = 9;
                        }

                        memset(text_16seg, ' ', sizeof(text_16seg));
                        memcpy(text_16seg, buf, n);
                    }

                    break;
                }

                case '3': {
                    int state;
                    printf("Enter neon state (0/1): ");
                    int n = scanf("%d", &state);

                    if (n > 0) {
                        top_neons = state > 0;
                    }

                    break;
                }

                case '4': {
                    char buf[9];
                    printf("Enter top lamp state, chain of 0/1s: ");
                    int n = scanf("%8s", buf);

                    if (n > 0) {
                        top_lamps = 0;
                        for (int i = 0; i < 8; ++i) {
                            if (buf[i] == '1') {
                                top_lamps |= (1 << i);
                            }
                        }
                    }

                    break;
                }

                case '5': {
                    all_on = true;
                    break;
                }

                case '6': {
                    all_on = false;
                    break;
                }

                case '0':
                default:
                    break;
            }
        }
    }

    system("cls");

    _all_lights_off_shutdown();

    bt_io_iidx_fini();

    bt_io_iidx_api_clear();
    module_io_free(&module_io_iidx);

    return 0;
}