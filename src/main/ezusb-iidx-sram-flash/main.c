#include <windows.h>

#include <stdio.h>
#include <string.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"

#include "iface-core/log.h"

#include "ezusb-iidx/sram.h"
#include "ezusb/ezusb.h"

#include "util/fs.h"

int main(int argc, char **argv)
{
    HANDLE handle;
    void *buffer;
    size_t size;
    bool res;

    if (argc < 3) {
        printf("ezusb-iidx-sram-flash for EZUSB hardware, e.g. IIDX C02 IO, "
               "build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV) "\n"
                                                                            "Usage: %s [ezusb dev path] [sram bin]\n",
               argv[0]);
        return -1;
    }

    core_log_bt_core_api_set();

    core_log_bt_ext_init_with_stderr();
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);

    log_info("Opening ezusb '%s'...", argv[1]);

    handle = ezusb_open(argv[1]);

    if (handle == INVALID_HANDLE_VALUE) {
        log_fatal("Cannot open ezusb device: %s", argv[1]);
        return -2;
    }

    if (!file_load(argv[2], &buffer, &size, false)) {
        log_fatal("Loading file %s failed", argv[2]);
        ezusb_close(handle);
        return -3;
    }

    log_info("Initializing SRAM...");

    res = ezusb_iidx_sram_init(handle, buffer, size);

    if (!res) {
        log_fatal("Downloading SRAM data failed");
        ezusb_close(handle);
        return -4;
    }

    log_misc("Data download successful\n");
    ezusb_close(handle);

    return 0;
}