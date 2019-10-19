#ifndef EZUSB2_EMU_DEVICE_H
#define EZUSB2_EMU_DEVICE_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "ezusb-emu/msg.h"

#include "hook/iohook.h"

/**
 * Hook IO functions to intercept with EZUSB FX2 (IIDX/Pop'n IO2) communication
 * and detour to our emulation code.
 *
 * @param msg_hook Hook functions to dispatch ezusb interrupt and bulk device
 *        messages to
 */
void ezusb2_emu_device_hook_init(struct ezusb_emu_msg_hook *msg_hook);

/**
 * Cleanup the hooked IO functions.
 */
void ezusb2_emu_device_hook_fini(void);

/**
 * Iohook interface.
 */
HRESULT ezusb2_emu_device_dispatch_irp(struct irp *irp);

#endif
