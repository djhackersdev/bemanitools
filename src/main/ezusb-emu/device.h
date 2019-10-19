#ifndef EZUSB_EMU_DEVICE_H
#define EZUSB_EMU_DEVICE_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "ezusb-emu/msg.h"

#include "hook/iohook.h"

/**
 * Hook IO functions to intercept with ezusb (IIDX C02) communication and
 * detour to our emulation code.
 *
 * @param msg_hook Hook functions to dispatch ezusb interrupt and bulk device
 *        messages to
 */
void ezusb_emu_device_hook_init(struct ezusb_emu_msg_hook *msg_hook);

/**
 * Cleanup the hooked IO functions.
 */
void ezusb_emu_device_hook_fini(void);

/**
 * Iohook interface.
 */
HRESULT ezusb_emu_device_dispatch_irp(struct irp *irp);

#endif
