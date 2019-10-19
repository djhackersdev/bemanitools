#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "ezusb/ezusb.h"
#include "ezusb/util.h"

#include "util/log.h"

static int info()
{
    HANDLE handle;
    struct ezusb_ident ident;

    log_misc("Opening device %s", EZUSB_DEVICE_PATH);

    handle = ezusb_open(EZUSB_DEVICE_PATH);

    if (handle == INVALID_HANDLE_VALUE) {
        log_fatal("Could not open device %s", EZUSB_DEVICE_PATH);
        return -3;
    }

    if (!ezusb_get_ident(handle, &ident)) {
        log_fatal("Getting ident information failed");
        return -4;
    }

    ezusb_close(handle);

    log_info("Ident information format: <hex vid>,<hex pid>,<str dev name>");
    printf("%04X,%04X,%s\n", ident.vid, ident.pid, ident.name);

    return 0;
}

static int flash(const char *fw_path)
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

    log_misc("Opening device %s", EZUSB_DEVICE_PATH);

    handle = ezusb_open(EZUSB_DEVICE_PATH);

    if (handle == INVALID_HANDLE_VALUE) {
        ezusb_firmware_free(fw);
        log_fatal("Could not open device %s", EZUSB_DEVICE_PATH);
        return -7;
    }

    log_misc("Flashing firmware...");

    if (!ezusb_download_firmware(handle, fw)) {
        log_fatal("Flashing firmware failed");
        return -8;
    } else {
        log_info("Flashing firmware successful");
    }

    ezusb_close(handle);
    ezusb_firmware_free(fw);

    return 0;
}

static void usage(const char *argv0)
{
    printf("ezusb-tool for EZUSB hardware, e.g. IIDX C02 IO, build " __DATE__ " " __TIME__
           ", gitrev " STRINGIFY(GITREV) "\n"
                                         "Usage: %s [cmd] ...\n"
                                         "Available commands:\n"
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

    log_to_writer(log_writer_stderr, NULL);

    if (!strcmp(argv[arg_pos], "info")) {
        return info();
    } else if (!strcmp(argv[arg_pos], "flash")) {
        arg_pos++;

        if (arg_pos >= argc) {
            printf(
                "Usage: flash [fw_path]\n"
                "  fw_path: Path to firmware binary file to flash\n");
            return -1;
        }

        return flash(argv[arg_pos]);
    } else {
        usage(argv[0]);
        return -1;
    }
}