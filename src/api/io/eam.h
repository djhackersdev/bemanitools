#ifndef BT_API_IO_EAM_H
#define BT_API_IO_EAM_H

/* Card reader emulator API. You may replace the stock EAMIO.DLL supplied by
   Bemanitools with your own custom implementation, which should implement the
   interface contract defined in this header file. */

#include <stdbool.h>
#include <stdint.h>

/* Scan codes for the so-called "10 key" button panel on each card reader. Each
   scan code corresponds to a bit position within the 16-bit bitfield that you
   return from eam_io_get_keypad_state(). */

typedef enum bt_io_eam_keypad_scan_code {
    BT_IO_EAM_KEYPAD_SCAN_CODE_0 = 0,
    BT_IO_EAM_KEYPAD_SCAN_CODE_1 = 1,
    BT_IO_EAM_KEYPAD_SCAN_CODE_4 = 2,
    BT_IO_EAM_KEYPAD_SCAN_CODE_7 = 3,
    BT_IO_EAM_KEYPAD_SCAN_CODE_00 = 4,
    BT_IO_EAM_KEYPAD_SCAN_CODE_2 = 5,
    BT_IO_EAM_KEYPAD_SCAN_CODE_5 = 6,
    BT_IO_EAM_KEYPAD_SCAN_CODE_8 = 7,
    BT_IO_EAM_KEYPAD_SCAN_CODE_DECIMAL = 8,
    BT_IO_EAM_KEYPAD_SCAN_CODE_3 = 9,
    BT_IO_EAM_KEYPAD_SCAN_CODE_6 = 10,
    BT_IO_EAM_KEYPAD_SCAN_CODE_9 = 11,

    BT_IO_EAM_KEYPAD_COUNT = 12, /* Not an actual scan code */
} bt_io_eam_keypad_scan_code_t;

/* Emulating the sensors of a slotted card reader. The reader has one
   sensor at the front that detects if a card is getting inserted or
   if the card is not fully removed. When the back sensor is triggered
   the card is locked in the slot and its data is read. */

typedef enum bt_io_eam_sensor_state {
    BT_IO_EAM_SENSOR_STATE_FRONT = 0,
    BT_IO_EAM_SENSOR_STATE_BACK = 1,
} bt_io_eam_sensor_state_t;

/* Different commands for the (slotted) reader. The game triggers one
   of these actions and the card slot as to execute it. When non-slotted
   readers are emulated, these states are not used/set. */

typedef enum bt_io_eam_card_slot_cmd {
    BT_IO_EAM_CARD_SLOT_CMD_CLOSE = 0,
    BT_IO_EAM_CARD_SLOT_CMD_OPEN = 1,
    BT_IO_EAM_CARD_SLOT_CMD_EJECT = 2,
    BT_IO_EAM_CARD_SLOT_CMD_READ = 3,
} bt_io_eam_card_slot_cmd_t;

/* Emulating of the card type for new readers. */

typedef enum bt_io_eam_read_card_result {
    BT_IO_EAM_READ_CARD_RESULT_NONE = 0,
    BT_IO_EAM_READ_CARD_RESULT_ISO15696 = 1,
    BT_IO_EAM_READ_CARD_RESULT_FELICA = 2,
} bt_io_eam_read_card_result_t;

typedef struct bt_io_eam_config_api bt_io_eam_config_api_t;

typedef bool (*bt_io_eam_init_t)();
typedef void (*bt_io_eam_fini_t)();
typedef uint16_t (*bt_io_eam_keypad_state_get_t)(uint8_t unit_no);
typedef uint8_t (*bt_io_eam_sensor_state_get_t)(uint8_t unit_no);
typedef uint8_t (*bt_io_eam_card_read_t)(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes);
typedef bool (*bt_io_eam_card_slot_cmd_send_t)(uint8_t unit_no, uint8_t cmd);
typedef bool (*bt_io_eam_poll_t)(uint8_t unit_no);
typedef const bt_io_eam_config_api_t *(*bt_io_eam_config_api_get_t)();

typedef struct bt_io_eam_api {
   uint16_t version;

   struct {
         // Required to be implemented
         bt_io_eam_init_t init;
         bt_io_eam_fini_t fini;
         bt_io_eam_keypad_state_get_t keypad_state_get;
         bt_io_eam_sensor_state_get_t sensor_state_get;
         bt_io_eam_card_read_t card_read;
         bt_io_eam_card_slot_cmd_send_t card_slot_cmd_send;
         bt_io_eam_poll_t poll;
         bt_io_eam_config_api_get_t config_api_get;
   } v1;
} bt_io_eam_api_t;

#endif
