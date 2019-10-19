#include <stdio.h>
#include <string.h>

#include <windows.h>

#include "ezusb-iidx/fpga.h"
#include "ezusb/ezusb.h"

#include "util/fs.h"
#include "util/log.h"

int main(int argc, char **argv)
{
    HANDLE handle;
    void *buffer;
    size_t size;
    bool res;

    if (argc < 3) {
        printf("ezusb-iidx-fpga-flash for EZUSB hardware, e.g. IIDX C02 IO, "
               "build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV) "\n"
                                                                            "Usage: %s [v1/v2] [fpga fw bin]\n",
               argv[0]);
        return -1;
    }

    log_to_writer(log_writer_stderr, NULL);

    log_info("Opening ezusb '%s'...", EZUSB_DEVICE_PATH);

    handle = ezusb_open(EZUSB_DEVICE_PATH);

    if (handle == INVALID_HANDLE_VALUE) {
        log_fatal("Cannot open ezusb device: %s", EZUSB_DEVICE_PATH);
        return -2;
    }

    if (!file_load(argv[2], &buffer, &size, false)) {
        log_fatal("Loading file %s failed", argv[2]);
        ezusb_close(handle);
        return -3;
    }

    log_info("Downloading FPGA firmware...");

    if (!strcmp(argv[1], "v1")) {
        res = ezusb_iidx_fpga_v1_init(handle, buffer, size);
    } else {
        res = ezusb_iidx_fpga_v2_init(handle, buffer, size);
    }

    if (!res) {
        log_fatal("Downloading FPGA firmware failed");
        ezusb_close(handle);
        return -4;
    }

    log_misc("Firmware download successful");
    ezusb_close(handle);

    return 0;
}