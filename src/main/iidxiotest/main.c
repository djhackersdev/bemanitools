#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "bemanitools/iidxio.h"

#include "util/log.h"
#include "util/thread.h"

static uint8_t _fix_top_lamps_order(uint8_t top_lamps)
{
    uint8_t out;

    out = 0;

    out |= ((1 << 0) & top_lamps) > 0 ? (1 << IIDX_IO_TOP_LAMP_LEFT_RED) : 0;
    out |= ((1 << 1) & top_lamps) > 0 ? (1 << IIDX_IO_TOP_LAMP_LEFT_YELLOW) : 0;
    out |= ((1 << 2) & top_lamps) > 0 ? (1 << IIDX_IO_TOP_LAMP_LEFT_GREEN) : 0;
    out |= ((1 << 3) & top_lamps) > 0 ? (1 << IIDX_IO_TOP_LAMP_LEFT_BLUE) : 0;

    out |= ((1 << 4) & top_lamps) > 0 ? (1 << IIDX_IO_TOP_LAMP_RIGHT_BLUE) : 0;
    out |= ((1 << 5) & top_lamps) > 0 ? (1 << IIDX_IO_TOP_LAMP_RIGHT_GREEN) : 0;
    out |=
        ((1 << 6) & top_lamps) > 0 ? (1 << IIDX_IO_TOP_LAMP_RIGHT_YELLOW) : 0;
    out |= ((1 << 7) & top_lamps) > 0 ? (1 << IIDX_IO_TOP_LAMP_RIGHT_RED) : 0;

    return out;
}

static void _all_lights_off_shutdown()
{
    iidx_io_ep1_set_deck_lights(0);
    iidx_io_ep1_set_panel_lights(0);
    iidx_io_ep1_set_top_lamps(0);
    iidx_io_ep1_set_top_neons(false);

    // Depending on the IO, pushing the state to the actual outputs, e.g. lights
    // on/off can be a bit finicky. Do a few times to "enforce" the state
    for (uint8_t i = 0; i < 3; i++) {
        iidx_io_ep1_send();
        // Required to handle iidxio-ezusb specific quirks with flushing 16seg
        // text
        iidx_io_ep2_recv();
        iidx_io_ep3_write_16seg("         ");

        Sleep(10);
    }
}

/**
 * Tool to test your implementations of iidxio.
 */
int main(int argc, char **argv)
{
    log_to_writer(log_writer_stdout, NULL);

    iidx_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!iidx_io_init(crt_thread_create, crt_thread_join, crt_thread_destroy)) {
        printf("Initializing iidxio failed\n");
        return -1;
    }

    printf(">>> Initializing iidxio successful, press enter to continue <<<\n");

    if (getchar() != '\n') {
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
        if (!iidx_io_ep2_recv()) {
            printf("ERROR: Receive ep2 failed\n");
            return -2;
        }

        /* get inputs */
        input_sys = iidx_io_ep2_get_sys();
        input_panel = iidx_io_ep2_get_panel();
        input_keys = iidx_io_ep2_get_keys();
        turn_table[0] = iidx_io_ep2_get_turntable(0);
        turn_table[1] = iidx_io_ep2_get_turntable(1);
        for (uint8_t i = 0; i < 5; ++i) {
            slider[i] = iidx_io_ep2_get_slider(i);
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
            (input_panel >> IIDX_IO_PANEL_EFFECT) & 1,
            (input_sys >> IIDX_IO_SYS_TEST) & 1,

            (input_panel >> IIDX_IO_PANEL_P1_START) & 1,
            slider[0],
            slider[1],
            slider[2],
            slider[3],
            slider[4],
            (input_panel >> IIDX_IO_PANEL_P2_START) & 1,

            (input_panel >> IIDX_IO_PANEL_VEFX) & 1,
            (input_sys >> IIDX_IO_SYS_SERVICE) & 1,

            turn_table[0],
            (input_keys >> IIDX_IO_KEY_P1_2) & 1,
            (input_keys >> IIDX_IO_KEY_P1_4) & 1,
            (input_keys >> IIDX_IO_KEY_P1_6) & 1,
            (input_sys >> IIDX_IO_SYS_COIN),
            (input_keys >> IIDX_IO_KEY_P2_2) & 1,
            (input_keys >> IIDX_IO_KEY_P2_4) & 1,
            (input_keys >> IIDX_IO_KEY_P2_6) & 1,
            turn_table[1],
            (input_keys >> IIDX_IO_KEY_P1_1) & 1,
            (input_keys >> IIDX_IO_KEY_P1_3) & 1,
            (input_keys >> IIDX_IO_KEY_P1_5) & 1,
            (input_keys >> IIDX_IO_KEY_P1_7) & 1,

            (input_keys >> IIDX_IO_KEY_P2_1) & 1,
            (input_keys >> IIDX_IO_KEY_P2_3) & 1,
            (input_keys >> IIDX_IO_KEY_P2_5) & 1,
            (input_keys >> IIDX_IO_KEY_P2_7) & 1);

        /* set outputs */
        if (all_on) {
            deck_lights = 0x3FFF;
            panel_lights = 0xF;
            top_lamps = 0xFF;
            top_neons = true;
            memset(text_16seg, '*', 9);
        }

        iidx_io_ep1_set_deck_lights(deck_lights);
        iidx_io_ep1_set_panel_lights(panel_lights);
        iidx_io_ep1_set_top_lamps(_fix_top_lamps_order(top_lamps));
        iidx_io_ep1_set_top_neons(top_neons);

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

        if (!iidx_io_ep1_send()) {
            printf("ERROR: Sending ep1 failed\n");
            return -4;
        }

        if (!iidx_io_ep3_write_16seg(text_16seg)) {
            printf("ERROR: Sending ep3 failed\n");
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

    iidx_io_fini();

    return 0;
}