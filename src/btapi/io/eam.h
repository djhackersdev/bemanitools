#ifndef BT_IO_EAM_H
#define BT_IO_EAM_H

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
};

/* Emulating of the card type for new readers. */

typedef enum bt_io_eam_read_card_result {
    BT_IO_EAM_READ_CARD_RESULT_NONE = 0,
    BT_IO_EAM_READ_CARD_RESULT_ISO15696 = 1,
    BT_IO_EAM_READ_CARD_RESULT_FELICA = 2,
} bt_io_eam_read_card_result_t;

/* A private function pointer table returned by the stock EAMIO.DLL
   implementation and consumed by config.exe. The contents of this table are
   undocumented and subject to change without notice. */

typedef struct bt_io_eam_config_api bt_io_eam_config_api_t;

typedef bool (*bt_io_eam_init_t)();
typedef void (*bt_io_eam_fini_t)();
typedef uint16_t (*bt_io_eam_keypad_state_get_t)(uint8_t unit_no);
typedef uint8_t (*bt_io_eam_sensor_state_get_t)(uint8_t unit_no);
typedef uint8_t (*bt_io_eam_card_read_t)(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes);
typedef bool (*bt_io_eam_card_slot_cmd_send_t)(uint8_t unit_no, uint8_t cmd);
typedef bool (*bt_io_eam_poll_t)(uint8_t unit_no);
typedef const bt_io_eam_config_api_t *(*bt_io_eam_config_api_get)();

/* Initialize your card reader emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool bt_io_eam_init();

/* Shut down your card reader emulation DLL. */

void bt_io_eam_fini();

/* Return the state of the number pad on your reader. This function will be
   called frequently. See enum eam_io_keypad_scan_code above for the meaning of
   each bit within the return value.

   This function will be called even if the running game does not actually have
   a number pad on the real cabinet (e.g. Jubeat).

   unit_no is either 0 or 1. Games with only a single reader (jubeat, popn,
   drummania) will only use unit_no 0. */

uint16_t bt_io_eam_keypad_state_get(uint8_t unit_no);

/* Indicate which sensors (front and back) are triggered for a slotted reader
   (refer to enum). To emulate non-slotted readers, just set both sensors
   to on to indicate the card is in range of the reader. This function
   will be called frequently. */

uint8_t bt_io_eam_sensor_state_get(uint8_t unit_no);

/* Read a card ID. This function is only called when the return value of
   eam_io_get_sensor_state() changes from false to true, so you may take your
   time and perform file I/O etc, within reason. You must return exactly eight
   bytes into the buffer pointed to by card_id. */

uint8_t bt_io_eam_card_read(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes);

/* Send a command to the card slot. This is called by the game to execute
   certain actions on a slotted reader (refer to enum). When emulating
   wave pass readers, this is function is never called. */

bool bt_io_eam_card_slot_cmd_send(uint8_t unit_no, uint8_t cmd);

/* This function is called frequently. Update your device and states in here */

bool bt_io_eam_poll(uint8_t unit_no);

/* Return a pointer to an internal configuration API for use by config.exe.
   Custom implementations should return NULL. */

const bt_io_eam_config_api_t *bt_io_eam_config_api_get();

#endif
