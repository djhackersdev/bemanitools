#ifndef EZUSB2_POPN_EMU_MSG_H
#define EZUSB2_POPN_EMU_MSG_H

#include <stddef.h>
#include <stdint.h>

#include "hook/iohook.h"

#include "ezusb-emu/msg.h"

/**
 * Init the fully emulated pop'n msg backend for a EZUSB FX2 (IO2) board
 *
 * @return ezusb_emu_msg_hook structure with hook calls for ezusb msg
 *         dispatching
 */
struct ezusb_emu_msg_hook *ezusb2_popn_emu_msg_init(void);

#endif
