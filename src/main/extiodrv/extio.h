#ifndef EXTIODRV_EXTIO_H
#define EXTIODRV_EXTIO_H

#include <windows.h>

#include <stdbool.h>

#include "extio/cmd.h"

#include "device.h"

enum extiodrv_extio_sensor_read_mode {
    EXTIODRV_EXTIO_SENSOR_READ_MODE_ALL = 0,
    EXTIODRV_EXTIO_SENSOR_READ_MODE_UP = 1,
    EXTIODRV_EXTIO_SENSOR_READ_MODE_DOWN = 2,
    EXTIODRV_EXTIO_SENSOR_READ_MODE_LEFT = 3,
    EXTIODRV_EXTIO_SENSOR_READ_MODE_RIGHT = 4
};

struct extiodrv_extio_pad_lights {
    bool up;
    bool down;
    bool left;
    bool right;
};

/**
 * Execute a data transfer (write + read) to the EXTIO device
 *
 * @param handle A valid and opened handle to an EXTIO device
 * @param sensor_read_mode The sensor read mode to set on the EXTIO device.
 *        This only needs to be set once that input reads return the configured
 *        sensor read setting.
 * @param pad_lights Pad light ouptut state to set
 * @param neons Neon output state to set
 * @return S_OK on success, other HRESULT value on error
 */
HRESULT extiodrv_extio_transfer(
    HANDLE handle,
    enum extiodrv_extio_sensor_read_mode sensor_read_mode,
    const struct extiodrv_extio_pad_lights
        pad_lights[EXTIO_PAD_LIGHT_MAX_PLAYERS],
    bool neons);

#endif