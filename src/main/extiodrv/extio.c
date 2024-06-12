#define LOG_MODULE "extiodrv-extio"

#include "extio.h"

#include "iface-core/log.h"

// static uint8_t _extiodrv_extio_sensor_read_mode_map[5] = {
//     1, // all
//     2, // up
//     3, // down
//     4, // left
//     5, // right
// };

HRESULT extiodrv_extio_transfer(
    HANDLE handle,
    enum extiodrv_extio_sensor_read_mode sensor_read_mode,
    const struct extiodrv_extio_pad_lights
        pad_lights[EXTIO_PAD_LIGHT_MAX_PLAYERS],
    bool neons)
{
    HRESULT hr;
    struct extio_cmd_write write;
    struct extio_cmd_read read;
    size_t bytes_written;
    size_t bytes_read;
    const uint8_t *raw;

    log_assert(handle != INVALID_HANDLE_VALUE);

    memset(&write, 0, sizeof(write));

    // TODO this doesn't seem to work, yet
    // write.sensor_read_mode =
    // 0;//_extiodrv_extio_sensor_read_mode_map[sensor_read_mode]; 0 = all 1 =
    // mess? 2 = mess? 3 = mess? 4 = mess? 5 = mess? 6 = right sensor only 7 =
    // right sensor only 8 = right sensor only
    write.sensor_read_mode = 0;

    for (uint8_t i = 0; i < EXTIO_PAD_LIGHT_MAX_PLAYERS; i++) {
        write.pad_lights[i].up = pad_lights[i].up ? 1 : 0;
        write.pad_lights[i].down = pad_lights[i].down ? 1 : 0;
        write.pad_lights[i].left = pad_lights[i].left ? 1 : 0;
        write.pad_lights[i].right = pad_lights[i].right ? 1 : 0;
    }

    write.neons = neons ? 1 : 0;

    // This MUST be set but only on the p1 packet
    // Not setting it or also setting it on the p2 packet results in the EXTIO
    // not responding at all
    write.pad_lights[0].unknown_80 = 1;

    write.checksum = extio_cmd_checksum(&write);

    raw = (uint8_t *) &write;

    log_misc("Raw write paket: %X %X %X %X", raw[0], raw[1], raw[2], raw[3]);

    hr = extiodrv_device_write(handle, &write, sizeof(write), &bytes_written);

    if (FAILED(hr)) {
        log_warning("Writing extio device failed");
        return hr;
    }

    if (bytes_written != sizeof(write)) {
        log_warning(
            "Writing extio device failed, expected bytes %d != actual bytes %d",
            (uint32_t) sizeof(write),
            (uint32_t) bytes_written);
        return HRESULT_FROM_WIN32(ERROR_BAD_LENGTH);
    }

    // Read loop to keep reading until a full message is read
    // There are various occasions, especially if polling from the host side is
    // faster than the device can put the response to the wire. this can result
    // in empty reads
    while (true) {
        hr = extiodrv_device_read(handle, &read, sizeof(read), &bytes_read);

        if (FAILED(hr)) {
            log_warning("Reading extio device failed");
            return hr;
        }

        if (bytes_read < sizeof(read)) {
            log_misc("Empty read, retry read");
            continue;
        }

        if (bytes_read != sizeof(read)) {
            log_warning(
                "Reading extio device failed, expected bytes %d != actual "
                "bytes %d",
                (uint32_t) sizeof(read),
                (uint32_t) bytes_read);
            return HRESULT_FROM_WIN32(ERROR_BAD_LENGTH);
        }

        break;
    }

    if (read.status != EXTIO_STATUS_OK) {
        log_warning("Status not ok: 0x%X", read.status);
        return HRESULT_FROM_WIN32(ERROR_GEN_FAILURE);
    }

    return S_OK;
}