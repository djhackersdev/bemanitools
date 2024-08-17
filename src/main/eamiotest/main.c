#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"

#include "iface-core/log.h"
#include "iface-io/eam.h"

#include "module/io-ext.h"

static void _eamiotest_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

/**
 * Tool to test your implementations of eamio.
 */
int main(int argc, char **argv)
{
    module_io_t *module_io_eam;

    core_log_bt_core_api_set();

    core_log_bt_ext_init_with_stderr();
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);

    _eamiotest_io_eam_init(&module_io_eam);

    if (!bt_io_eam_init()) {
        printf("Initializing eamio failed\n");

        bt_io_eam_api_clear();
        module_io_free(&module_io_eam);

        return -1;
    }

    printf(">>> Initializing eamio successful, press enter to continue <<<\n");

    if (getchar() != '\n') {
        bt_io_eam_fini();

        bt_io_eam_api_clear();
        module_io_free(&module_io_eam);

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
            if (!bt_io_eam_poll(node)) {
                printf("ERROR: Polling node %d failed", node);
                return -2;
            }

            uint16_t keypad = bt_io_eam_keypad_state_get(node);
            uint16_t keypad_rise = ~keypad_prev[node] & keypad;

            uint8_t sensors = bt_io_eam_sensor_state_get(node);

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
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_7)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_8)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_9)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_4)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_5)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_6)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_1)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_2)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_3)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_0)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_00)) > 0,
                (keypad & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_DECIMAL)) > 0,
                (sensors & (1 << BT_IO_EAM_SENSOR_STATE_FRONT)) > 0,
                (sensors & (1 << BT_IO_EAM_SENSOR_STATE_BACK)) > 0,
                card[node][0],
                card[node][1],
                card[node][2],
                card[node][3],
                card[node][4],
                card[node][5],
                card[node][6],
                card[node][7]);

            if (sensors & (1 << BT_IO_EAM_SENSOR_STATE_BACK)) {
                bt_io_eam_card_slot_cmd_send(
                    node, BT_IO_EAM_CARD_SLOT_CMD_CLOSE);
                bt_io_eam_poll(node);
                bt_io_eam_card_slot_cmd_send(
                    node, BT_IO_EAM_CARD_SLOT_CMD_READ);
                bt_io_eam_poll(node);
                bt_io_eam_card_read(node, card[node], 8);
            }

            if (sensors == 0) {
                memset(card[node], 0, 8);

                bt_io_eam_card_slot_cmd_send(
                    node, BT_IO_EAM_CARD_SLOT_CMD_CLOSE);
                bt_io_eam_poll(node);
                bt_io_eam_card_slot_cmd_send(
                    node, BT_IO_EAM_CARD_SLOT_CMD_OPEN);
            }

            if (keypad_rise & (1 << BT_IO_EAM_KEYPAD_SCAN_CODE_DECIMAL)) {
                bt_io_eam_card_slot_cmd_send(
                    node, BT_IO_EAM_CARD_SLOT_CMD_EJECT);
            }

            keypad_prev[node] = keypad;
        }

        /* avoid CPU banging */
        Sleep(5);
    }

    bt_io_eam_fini();

    bt_io_eam_api_clear();
    module_io_free(&module_io_eam);

    return 0;
}