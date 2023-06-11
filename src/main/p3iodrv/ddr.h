#ifndef P3IODRV_DDR_H
#define P3IODRV_DDR_H

#include <stdbool.h>
#include <stddef.h>
#include <windows.h>

#include "p3io/cmd.h"
#include "p3io/ddr.h"

#define P3IODRV_DDR_VERSION_STR_LEN 4

/**
 * Initialize the p3io device for operation.
 *
 * This call is also somewhat referred to as "set mode" in the game, though it
 * is part of a larger setup routine.
 *
 * If the p3io device is not initialized, the menu buttons (left, right, start)
 * on (a SD) cabinet will periodically blink. Once this function is called, this
 * blinking stops indicating the p3io device is initialized.
 *
 * Other p3io commands might work (partially) even if this function is not
 * called. However, it is recommended to call this first after every p3io
 * device reset.
 *
 * @param handle A handle to an opened p3io device.
 * @result S_OK on success, any other HRESULT value on error
 */
HRESULT p3iodrv_ddr_init(HANDLE handle);

/**
 * Get the DDR specific version information from the p3io device.
 *
 * @param handle A handle to an opened p3io device.
 * @param str Pointer to a buffer to write the version string to
 * @param major Pointer to a uint32 to write the major version number to
 * @param minor Pointer to a uint32 to write the minor version number to
 * @param patch Poitner to a uint32 to ewrite the patch version number to
 * @result S_OK on success, any other HRESULT value on error
 */
HRESULT p3iodrv_ddr_get_version(
    HANDLE handle,
    char str[P3IODRV_DDR_VERSION_STR_LEN],
    uint32_t *major,
    uint32_t *minor,
    uint32_t *patch);

/**
 * Enable/disable the device side watchdog. The watchdog will reset the device
 * if it is not reset periodically.
 *
 * If you stop sending commands to the p3io, it will trigger after about 5-7
 * seconds of not receiving anything. You can notice the watchdog triggered when
 * the menu buttons start to blink again which indicates the device is not
 * initialized (anymore).
 *
 * The watchdog seems to expect that you periodically send the "get version"
 * command to reset it and keep the p3io alive (p3iodrv_ddr_get_version).
 *
 * @param handle A handle to an opened p3io device.
 * @param enable true to enable the watchdog, false to disable
 * @result S_OK on success, any other HRESULT value on error
 */
HRESULT p3iodrv_ddr_set_watchdog(HANDLE handle, bool enable);

/**
 * Get the current state of the dip switches from the p3io.
 *
 * @param handle A handle to an opened p3io device.
 * @param state Pointer to a uint8 to write the resulting dip switch state to
 * @result S_OK on success, any other HRESULT value on error
 */
HRESULT p3iodrv_ddr_get_dipsw(HANDLE handle, uint8_t *state);

/**
 * Get the detected cabinet type from the p3io.
 *
 * @param handle A handle to an opened p3io device.
 * @param type Pointer to a enum p3io_cab_type to write the resulting state to
 * @result S_OK on success, any other HRESULT value on error
 */
HRESULT p3iodrv_ddr_get_cab_type(HANDLE handle, enum p3io_cab_type *type);

/**
 * Get the detected video frequency of the monitor from the p3io.
 *
 * @param handle A handle to an opened p3io device.
 * @param type Pointer to a enum p3io_video_freq to write the resulting state to
 * @result S_OK on success, any other HRESULT value on error
 */
HRESULT p3iodrv_ddr_get_video_freq(HANDLE handle, enum p3io_video_freq *freq);

/**
 * Get the current state of the JAMMA edge/input from the p3io.
 *
 * @param handle A handle to an opened p3io device.
 * @param type Pointer to a struct p3io_ddr_jamma to write the resulting state
 * to
 * @result S_OK on success, any other HRESULT value on error
 */
HRESULT p3iodrv_ddr_get_jamma(HANDLE handle, struct p3io_ddr_jamma *jamma);

/**
 * Set (some cabinet) light output state on the p3io.
 *
 * @param handle A handle to an opened p3io device.
 * @param type Pointer to a struct p3io_ddr_output with the light output data to
 *        set on the p3io
 * @result S_OK on success, any other HRESULT value on error
 */
HRESULT
p3iodrv_ddr_set_outputs(HANDLE handle, const struct p3io_ddr_output *state);

#endif