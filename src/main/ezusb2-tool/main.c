#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"

#include "iface-core/log.h"

#include "ezusb/util.h"
#include "ezusb2/ezusb2.h"

static int scan()
{
    char *path;

    log_misc("Scanning...");

    path = ezusb2_find(&EZUSB2_GUID);

    if (!path) {
        log_fatal("Could not find a connected ezusb fx2 device.\n");
        return -2;
    } else {
        printf("%s\n", path);
        return 0;
    }
}

static int info(const char *dev_path)
{
    HANDLE handle;
    struct ezusb_ident ident;

    log_misc("Opening device %s", dev_path);

    handle = ezusb2_open(dev_path);

    if (handle == INVALID_HANDLE_VALUE) {
        log_fatal("Could not open device %s", dev_path);
        return -3;
    }

    if (!ezusb2_get_ident(handle, &ident)) {
        log_fatal("Getting ident information failed");
        return -4;
    }

    ezusb2_close(handle);

    log_info("Ident information format: <hex vid>,<hex pid>,<str dev name>");
    printf("%04X,%04X,%s\n", ident.vid, ident.pid, ident.name);

    return 0;
}

static int flash(const char *dev_path, const char *fw_path)
{
    HANDLE handle;
    struct ezusb_firmware *fw;

    fw = ezusb_firmware_load(fw_path);

    if (!fw) {
        log_fatal("Loading firmware from file '%s' failed", fw_path);
        return -5;
    }

    log_misc(
        "Loaded firmware, crc 0x%X, segments %d", fw->crc, fw->segment_count);

    if (ezusb_firmware_crc(fw) != fw->crc) {
        log_fatal("Firmware CRC check failed");
        return -6;
    }

    log_misc("Opening device %s", dev_path);

    handle = ezusb2_open(dev_path);

    if (handle == INVALID_HANDLE_VALUE) {
        ezusb_firmware_free(fw);
        log_fatal("Could not open device %s", dev_path);
        return -7;
    }

    log_misc("Flashing firmware...");

    if (!ezusb2_download_firmware(handle, fw)) {
        log_fatal("Flashing firmware failed");
        return -8;
    } else {
        log_info("Flashing firmware successful");
    }

    ezusb2_close(handle);
    ezusb_firmware_free(fw);

    return 0;
}

static void usage(const char *argv0)
{
    printf("ezusb2-tool for EZUSB FX2 hardware, e.g. IIDX/Pop'n IO2, build " __DATE__ " " __TIME__
           ", gitrev " STRINGIFY(GITREV) "\n"
                                         "Usage: %s [cmd] ...\n"
                                         "Available commands:\n"
                                         "  scan: Scan for connected EZUSB FX2 devices and output the "
                                         "device path\n"
                                         "  info: Get basic information (vid, pid, name) of a connected "
                                         "device\n"
                                         "  flash: Flash a firmware binary\n",
           argv0);
}

int main(int argc, char **argv)
{
    int arg_pos;

    if (argc < 2) {
        usage(argv[0]);
        return -1;
    }

    arg_pos = 1;

    core_log_bt_core_api_set();

    core_log_bt_ext_init_with_stderr();
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);

    if (!strcmp(argv[arg_pos], "scan")) {
        return scan();
    } else if (!strcmp(argv[arg_pos], "info")) {
        arg_pos++;

        if (arg_pos >= argc) {
            printf(
                "Usage: info [dev_path]\n"
                "  dev_path: Device path of device to query. Use the 'scan' "
                "command to aquire a device path of a connected device\n");
            return -1;
        }

        return info(argv[arg_pos]);
    } else if (!strcmp(argv[arg_pos], "flash")) {
        arg_pos++;

        if (arg_pos + 1 >= argc) {
            printf(
                "Usage: flash [dev_path] [fw_path]\n"
                "  dev_path: Device path of device to flash. Use the 'scan' "
                "command to aquire a device path of a connected device\n"
                "  fw_path: Path to firmware binary file to flash\n");
            return -1;
        }

        return flash(argv[arg_pos], argv[arg_pos + 1]);
    } else {
        usage(argv[0]);
        return -1;
    }
}