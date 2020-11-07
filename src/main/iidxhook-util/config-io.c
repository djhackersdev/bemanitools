#include "cconfig/cconfig-util.h"

#include "iidxhook-util/config-io.h"

#include "util/log.h"

#define IIDXHOOK_UTIL_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY \
    "io.disable_card_reader_emu"
#define IIDXHOOK_UTIL_CONFIG_IO_DISABLE_IO_EMU_KEY "io.disable_io_emu"

#define IIDXHOOK_UTIL_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE false
#define IIDXHOOK_UTIL_CONFIG_IO_DEFAULT_DISABLE_IO_EMU_VALUE false

void iidxhook_config_io_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        IIDXHOOK_UTIL_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY,
        IIDXHOOK_UTIL_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE,
        "Disable card reader emulation and enable usage of real card reader "
        "hardware on COM1 (for games supporting slotted/wavepass readers)");

    cconfig_util_set_bool(
        config,
        IIDXHOOK_UTIL_CONFIG_IO_DISABLE_IO_EMU_KEY,
        IIDXHOOK_UTIL_CONFIG_IO_DEFAULT_DISABLE_IO_EMU_VALUE,
        "Disable ezusb IO emulation and enable usage of real ezusb1/2 IO hardware");
}

void iidxhook_config_io_get(
    struct iidxhook_config_io *config_io, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_UTIL_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY,
            &config_io->disable_card_reader_emu,
            IIDXHOOK_UTIL_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_UTIL_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY,
            IIDXHOOK_UTIL_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_UTIL_CONFIG_IO_DISABLE_IO_EMU_KEY,
            &config_io->disable_io_emu,
            IIDXHOOK_UTIL_CONFIG_IO_DEFAULT_DISABLE_IO_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_UTIL_CONFIG_IO_DISABLE_IO_EMU_KEY,
            IIDXHOOK_UTIL_CONFIG_IO_DEFAULT_DISABLE_IO_EMU_VALUE);
    }
}
