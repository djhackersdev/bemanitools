#include "cconfig/cconfig-util.h"

#include "iidxhook9/config-io.h"

#include "util/log.h"

#define IIDXHOOK9_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY \
    "io.disable_card_reader_emu"
#define IIDXHOOK9_CONFIG_IO_DISABLE_BIO2_EMU_KEY "io.disable_bio2_emu"
#define IIDXHOOK9_CONFIG_IO_DISABLE_POLL_LIMITER_KEY "io.disable_poll_limiter"
#define IIDXHOOK9_CONFIG_IO_LIGHTNING_MODE_KEY "io.lightning_mode"
#define IIDXHOOK9_CONFIG_IO_DISABLE_CAMS_KEY "io.disable_cams"
#define IIDXHOOK9_CONFIG_IO_DISABLE_FILE_HOOKS_KEY "io.disable_file_hooks"
#define IIDXHOOK9_CONFIG_IO_TT_MULTIPLIER_KEY "io.tt_multiplier"

#define IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE false
#define IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_BIO2_EMU_VALUE false
#define IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_POLL_LIMITER_VALUE false
#define IIDXHOOK9_CONFIG_IO_DEFAULT_LIGHTNING_MODE_VALUE false
#define IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_CAMS_VALUE false
#define IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_FILE_HOOKS_VALUE false
#define IIDXHOOK9_CONFIG_IO_DEFAULT_TT_MULTIPLIER_VALUE 1.0

void iidxhook9_config_io_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        IIDXHOOK9_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY,
        IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE,
        "Disable card reader emulation and enable usage of real card reader "
        "hardware on COM1 (for games supporting slotted readers)");

    cconfig_util_set_bool(
        config,
        IIDXHOOK9_CONFIG_IO_DISABLE_BIO2_EMU_KEY,
        IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_BIO2_EMU_VALUE,
        "Disable BIO2 emulation and enable usage of real BIO2 hardware");

    cconfig_util_set_bool(
        config,
        IIDXHOOK9_CONFIG_IO_DISABLE_POLL_LIMITER_KEY,
        IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_POLL_LIMITER_VALUE,
        "Disables the poll limiter, warning very high CPU usage may arise");

    cconfig_util_set_bool(
        config,
        IIDXHOOK9_CONFIG_IO_LIGHTNING_MODE_KEY,
        IIDXHOOK9_CONFIG_IO_DEFAULT_LIGHTNING_MODE_VALUE,
        "Lightning cab mode (requires additional IO emulation)");

    cconfig_util_set_bool(
        config,
        IIDXHOOK9_CONFIG_IO_DISABLE_CAMS_KEY,
        IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_CAMS_VALUE,
        "Disable camera connection");

    cconfig_util_set_bool(
        config,
        IIDXHOOK9_CONFIG_IO_DISABLE_FILE_HOOKS_KEY,
        IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_FILE_HOOKS_VALUE,
        "Disables the built in file hooks, requiring manual file creation");

    cconfig_util_set_float(
        config,
        IIDXHOOK9_CONFIG_IO_TT_MULTIPLIER_KEY,
        IIDXHOOK9_CONFIG_IO_DEFAULT_TT_MULTIPLIER_VALUE,
        "Turntable sensitivity multiplier (1.0 is default)");
}

void iidxhook9_config_io_get(
    struct iidxhook9_config_io *config_io, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK9_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY,
            &config_io->disable_card_reader_emu,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK9_CONFIG_IO_DISABLE_CARD_READER_EMU_KEY,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_CARD_READER_EMU_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK9_CONFIG_IO_DISABLE_BIO2_EMU_KEY,
            &config_io->disable_bio2_emu,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_BIO2_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK9_CONFIG_IO_DISABLE_BIO2_EMU_KEY,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_BIO2_EMU_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK9_CONFIG_IO_DISABLE_POLL_LIMITER_KEY,
            &config_io->disable_poll_limiter,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_POLL_LIMITER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK9_CONFIG_IO_DISABLE_POLL_LIMITER_KEY,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_POLL_LIMITER_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK9_CONFIG_IO_LIGHTNING_MODE_KEY,
            &config_io->lightning_mode,
            IIDXHOOK9_CONFIG_IO_DEFAULT_LIGHTNING_MODE_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK9_CONFIG_IO_LIGHTNING_MODE_KEY,
            IIDXHOOK9_CONFIG_IO_DEFAULT_LIGHTNING_MODE_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK9_CONFIG_IO_DISABLE_CAMS_KEY,
            &config_io->disable_cams,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_CAMS_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK9_CONFIG_IO_DISABLE_CAMS_KEY,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_CAMS_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK9_CONFIG_IO_DISABLE_FILE_HOOKS_KEY,
            &config_io->disable_file_hooks,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_FILE_HOOKS_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK9_CONFIG_IO_DISABLE_FILE_HOOKS_KEY,
            IIDXHOOK9_CONFIG_IO_DEFAULT_DISABLE_FILE_HOOKS_VALUE);
    }

    if (!cconfig_util_get_float(
            config,
            IIDXHOOK9_CONFIG_IO_TT_MULTIPLIER_KEY,
            &config_io->tt_multiplier,
            IIDXHOOK9_CONFIG_IO_DEFAULT_TT_MULTIPLIER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%f'",
            IIDXHOOK9_CONFIG_IO_TT_MULTIPLIER_KEY,
            IIDXHOOK9_CONFIG_IO_DEFAULT_TT_MULTIPLIER_VALUE);
    }
}
