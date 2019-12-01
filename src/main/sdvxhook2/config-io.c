#include "cconfig/cconfig-util.h"

#include "sdvxhook2/config-io.h"

#include "util/log.h"

#define SDVXHOOK2_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY \
    "io.disable_card_reader_emu"
#define SDVXHOOK2_CONFIG_IO_DISABLE_BIO2_EMU_KEY "io.disable_bio2_emu"
#define SDVXHOOK2_CONFIG_IO_DISABLE_POLL_LIMITER_KEY "io.disable_poll_limiter"

#define SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE false
#define SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_BIO2_EMU_VALUE false
#define SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_POLL_LIMITER_VALUE false

void sdvxhook2_config_io_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        SDVXHOOK2_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY,
        SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE,
        "Disable card reader emulation and enable usage of real card reader "
        "hardware on COM2");

    cconfig_util_set_bool(
        config,
        SDVXHOOK2_CONFIG_IO_DISABLE_BIO2_EMU_KEY,
        SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_BIO2_EMU_VALUE,
        "Disable BIO2 emulation and enable usage of real BIO2 hardware");

    cconfig_util_set_bool(
        config,
        SDVXHOOK2_CONFIG_IO_DISABLE_POLL_LIMITER_KEY,
        SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_POLL_LIMITER_VALUE,
        "Disables the poll limiter, warning very high CPU usage may arise");
}

void sdvxhook2_config_io_get(
    struct sdvxhook2_config_io *config_io, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            SDVXHOOK2_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY,
            &config_io->disable_card_reader_emu,
            SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXHOOK2_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY,
            SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            SDVXHOOK2_CONFIG_IO_DISABLE_BIO2_EMU_KEY,
            &config_io->disable_bio2_emu,
            SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_BIO2_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXHOOK2_CONFIG_IO_DISABLE_BIO2_EMU_KEY,
            SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_BIO2_EMU_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            SDVXHOOK2_CONFIG_IO_DISABLE_POLL_LIMITER_KEY,
            &config_io->disable_poll_limiter,
            SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_POLL_LIMITER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXHOOK2_CONFIG_IO_DISABLE_POLL_LIMITER_KEY,
            SDVXHOOK2_CONFIG_IO_DEFAULT_DISABLE_POLL_LIMITER_VALUE);
    }
}
