#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "bemanitools/eamio.h"

#include "util/log.h"
#include "util/thread.h"

/**
 * Tool to test your implementations of eamio.
 */
int main(int argc, char **argv)
{
    eam_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!eam_io_init(crt_thread_create, crt_thread_join, crt_thread_destroy)) {
        printf("Initializing eamio failed\n");
        return -1;
    }

    printf(">>> Initializing eamio successful, press enter to continue <<<\n");

    if (getchar() != '\n') {
        return 0;
    }

    uint16_t keypad_prev[2];
    uint8_t card[2][8];

    memset(keypad_prev, 0, sizeof(keypad_prev));
    memset(card, 0, sizeof(card));

    while (true) {
        system("cls");

        if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
            break;
        }

        for (uint8_t node = 0; node < 2; ++node) {
            if (!eam_io_poll(node)) {
                printf("ERROR: Polling node %d failed", node);
                return -2;
            }

            uint16_t keypad = eam_io_get_keypad_state(node);
            uint16_t keypad_rise = ~keypad_prev[node] & keypad;

            uint8_t sensors = eam_io_get_sensor_state(node);

            printf(
                "Press escape to quit\n"
                "Reader %d\n"
                "------------------\n"
                " |7: %d|8: %d|9: %d|\n"
                " |4: %d|5: %d|6: %d|\n"
                " |1: %d|2: %d|3: %d|\n"
                " |0: %d|O: %d|_: %d|\n"
                "------------------\n"
                "|front: %d back: %d|\n"
                "------------------\n"
                "|%02X%02X%02X%02X%02X%02X%02X%02X|\n"
                "------------------\n",
                node,
                (keypad & (1 << EAM_IO_KEYPAD_7)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_8)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_9)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_4)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_5)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_6)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_1)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_2)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_3)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_0)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_00)) > 0,
                (keypad & (1 << EAM_IO_KEYPAD_DECIMAL)) > 0,
                (sensors & (1 << EAM_IO_SENSOR_FRONT)) > 0,
                (sensors & (1 << EAM_IO_SENSOR_BACK)) > 0,
                card[node][0],
                card[node][1],
                card[node][2],
                card[node][3],
                card[node][4],
                card[node][5],
                card[node][6],
                card[node][7]);

            if (sensors & (1 << EAM_IO_SENSOR_BACK)) {
                eam_io_card_slot_cmd(node, EAM_IO_CARD_SLOT_CMD_CLOSE);
                eam_io_poll(node);
                eam_io_card_slot_cmd(node, EAM_IO_CARD_SLOT_CMD_READ);
                eam_io_poll(node);
                eam_io_read_card(node, card[node], 8);
            }

            if (sensors == 0) {
                memset(card[node], 0, 8);

                eam_io_card_slot_cmd(node, EAM_IO_CARD_SLOT_CMD_CLOSE);
                eam_io_poll(node);
                eam_io_card_slot_cmd(node, EAM_IO_CARD_SLOT_CMD_OPEN);
            }

            if (keypad_rise & (1 << EAM_IO_KEYPAD_DECIMAL)) {
                eam_io_card_slot_cmd(node, EAM_IO_CARD_SLOT_CMD_EJECT);
            }

            keypad_prev[node] = keypad;
        }

        /* avoid CPU banging */
        Sleep(5);
    }

    eam_io_fini();

    return 0;
}