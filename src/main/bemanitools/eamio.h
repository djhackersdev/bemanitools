#ifndef BEMANITOOLS_EAM_H
#define BEMANITOOLS_EAM_H

/* Card reader emulator API. You may replace the stock EAMIO.DLL supplied by
   Bemanitools with your own custom implementation, which should implement the
   interface contract defined in this header file. */

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/glue.h"

/* Scan codes for the so-called "10 key" button panel on each card reader. Each
   scan code corresponds to a bit position within the 16-bit bitfield that you
   return from eam_io_get_keypad_state(). */

enum eam_io_keypad_scan_code {
    EAM_IO_KEYPAD_0 = 0,
    EAM_IO_KEYPAD_1 = 1,
    EAM_IO_KEYPAD_4 = 2,
    EAM_IO_KEYPAD_7 = 3,
    EAM_IO_KEYPAD_00 = 4,
    EAM_IO_KEYPAD_2 = 5,
    EAM_IO_KEYPAD_5 = 6,
    EAM_IO_KEYPAD_8 = 7,
    EAM_IO_KEYPAD_DECIMAL = 8,
    EAM_IO_KEYPAD_3 = 9,
    EAM_IO_KEYPAD_6 = 10,
    EAM_IO_KEYPAD_9 = 11,

    EAM_IO_KEYPAD_COUNT = 12, /* Not an actual scan code */
};

/* Emulating the sensors of a slotted card reader. The reader has one
   sensor at the front that detects if a card is getting inserted or
   if the card is not fully removed. When the back sensor is triggered
   the card is locked in the slot and its data is read. */

enum eam_io_sensor_state {
    EAM_IO_SENSOR_FRONT = 0,
    EAM_IO_SENSOR_BACK = 1,
};

/* Different commands for the (slotted) reader. The game triggers one
   of these actions and the card slot as to execute it. When non-slotted
   readers are emulated, these states are not used/set. */

enum eam_io_card_slot_cmd {
    EAM_IO_CARD_SLOT_CMD_CLOSE = 0,
    EAM_IO_CARD_SLOT_CMD_OPEN = 1,
    EAM_IO_CARD_SLOT_CMD_EJECT = 2,
    EAM_IO_CARD_SLOT_CMD_READ = 3,
};

/* Emulating of the card type for new readers. */

enum eam_io_read_card_result {
    EAM_IO_CARD_NONE = 0,
    EAM_IO_CARD_ISO15696 = 1,
    EAM_IO_CARD_FELICA = 2,
};

/* A private function pointer table returned by the stock EAMIO.DLL
   implementation and consumed by config.exe. The contents of this table are
   undocumented and subject to change without notice. */

struct eam_io_config_api;

/* The first function that will be called on your DLL. You will be supplied
   with four function pointers that may be used to log messages to the game's
   log file. See comments in glue.h for further information. */

void eam_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

/* Initialize your card reader emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool eam_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);

/* Shut down your card reader emulation DLL. */

void eam_io_fini(void);

/* Return the state of the number pad on your reader. This function will be
   called frequently. See enum eam_io_keypad_scan_code above for the meaning of
   each bit within the return value.

   This function will be called even if the running game does not actually have
   a number pad on the real cabinet (e.g. Jubeat).

   unit_no is either 0 or 1. Games with only a single reader (jubeat, popn,
   drummania) will only use unit_no 0. */

uint16_t eam_io_get_keypad_state(uint8_t unit_no);

/* Indicate which sensors (front and back) are triggered for a slotted reader
   (refer to enum). To emulate non-slotted readers, just set both sensors
   to on to indicate the card is in range of the reader. This function
   will be called frequently. */

uint8_t eam_io_get_sensor_state(uint8_t unit_no);

/* Read a card ID. This function is only called when the return value of
   eam_io_get_sensor_state() changes from false to true, so you may take your
   time and perform file I/O etc, within reason. You must return exactly eight
   bytes into the buffer pointed to by card_id. */

uint8_t eam_io_read_card(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes);

/* Send a command to the card slot. This is called by the game to execute
   certain actions on a slotted reader (refer to enum). When emulating
   wave pass readers, this is function is never called. */

bool eam_io_card_slot_cmd(uint8_t unit_no, uint8_t cmd);

/* This function is called frequently. Update your device and states in here */

bool eam_io_poll(uint8_t unit_no);

/* Return a pointer to an internal configuration API for use by config.exe.
   Custom implementations should return NULL. */

const struct eam_io_config_api *eam_io_get_config_api(void);

#endif
