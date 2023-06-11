#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "extiodrv/extio.h"

#include "util/log.h"

int main(int argc, char **argv)
{
    HRESULT hr;
    const char *port;
    HANDLE handle;
    struct extiodrv_extio_pad_lights pad_lights[EXTIO_PAD_LIGHT_MAX_PLAYERS];

    if (argc < 2) {
        fprintf(stderr, "Basic functional test of EXTIO\n");
        fprintf(stderr, "Usage: %s COM_PORT\n", argv[0]);
        fprintf(stderr, "  COM_PORT: For example COM1\n");
    }

    log_to_writer(log_writer_stderr, NULL);
    log_set_level(LOG_LEVEL_MISC);

    port = argv[1];

    hr = extiodrv_device_open(port, &handle);

    if (FAILED(hr)) {
        fprintf(stderr, "Opening extio at port '%s' failed: %lX\n", port, hr);
        return -1;
    }

    memset(&pad_lights, 0, sizeof(pad_lights));

    hr = extiodrv_extio_transfer(
        handle, EXTIODRV_EXTIO_SENSOR_READ_MODE_ALL, pad_lights, false);

    if (FAILED(hr)) {
        fprintf(stderr, "Extio transfer failed: %lX\n", hr);
        return -1;
    }

    hr = extiodrv_device_close(&handle);

    if (FAILED(hr)) {
        fprintf(stderr, "Closing extio failed: %lX", hr);
        return -1;
    }

    return 0;
}