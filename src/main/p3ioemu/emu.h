#ifndef P3IO_EMU_H
#define P3IO_EMU_H

#include <windows.h>

#include <stddef.h>
#include <stdint.h>

#include "hook/iohook.h"

/**
 * Enum for available video frequencies to select from. Depending on the
 * game, this feature is used or unused.
 */
enum p3io_video_freq {
    P3IO_VIDEO_FREQ_15KHZ = 0,
    P3IO_VIDEO_FREQ_31KHZ = 1,
};

/**
 * Enum for available cabinet/display types to select from. Depending on the
 * game, this feature is used or unused.
 */
enum p3io_cab_type {
    P3IO_CAB_TYPE_SD = 0,
    P3IO_CAB_TYPE_HD = 1,
};

/**
 * P3IO operation dispatching table. Many operations are optional and not used
 * by every game that's running on that type of hardware which means you can
 * ommit hooking up implementations for them.
 */
struct p3io_ops {
    void (*close)(void *ctx);
    HRESULT (*read_jamma)(void *ctx, uint32_t *state);
    HRESULT (*set_outputs)(void *ctx, uint32_t state);
    HRESULT (*get_dipsw)(void *ctx, uint8_t *state);
    HRESULT (*get_cab_type)(void *ctx, enum p3io_cab_type *type);
    HRESULT (*get_video_freq)(void *ctx, enum p3io_video_freq *freq);
    HRESULT (*get_coinstock)(void *ctx, uint16_t *slots, size_t nslots);
    HRESULT(*get_roundplug)
    (void *ctx, uint8_t plug_id, uint8_t *rom, uint8_t *eeprom);
};

/**
 * Initialize the p3io emulation backend.
 *
 * @param ops Dispatch table with operation hooks for your target game.
 * @param ctx A context which is passed along with every call to a function of
 *            your provided dispatch table.
 */
void p3io_emu_init(const struct p3io_ops *ops, void *ctx);

/**
 * Shutdown the p3io emulation backend.
 */
void p3io_emu_fini(void);

/**
 * p3io emulation dispatch irp call. This needs to be hooked to the iohook
 * module in order to drive the emulation backend with detoured system calls.
 */
HRESULT p3io_emu_dispatch_irp(struct irp *irp);

#endif
