#ifndef EZUSB_IIDX_EMU_MSG_H
#define EZUSB_IIDX_EMU_MSG_H

#include <stddef.h>
#include <stdint.h>

#include "hook/iohook.h"

#include "ezusb-emu/msg.h"

/**
 * Init the fully emulated IIDX msg backend for a EZUSB (C02) board. This 
 * activates the "old" V1 node handling backend which was used on iidx 09 to 13.
 * On iidx14, the firmware was updated and removed nodes that were not used
 * anymore, e.g. serial card reader handling.
 * 
 * @return ezusb_emu_msg_hook structure with hook calls for ezusb msg 
 *         dispatching
 */
struct ezusb_emu_msg_hook* ezusb_iidx_emu_msg_init(void);

/**
 * Init the fully emulated IIDX msg backend for a EZUSB (C02) board. This 
 * activates the "newer" V2 node handling backend which was used from iidx 14
 * onwards. The firmware was updated and removed nodes that were not used
 * anymore, e.g. serial card reader handling.
 * 
 * @return ezusb_emu_msg_hook structure with hook calls for ezusb msg 
 *         dispatching
 */
struct ezusb_emu_msg_hook* ezusb_iidx_emu_msg_v2_init(void);

#endif
