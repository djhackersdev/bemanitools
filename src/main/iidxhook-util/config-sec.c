#include <stdio.h>
#include <string.h>

#include "core/config-ext.h"

#include "iface-core/log.h"

#include "iidxhook-util/config-sec.h"

#include "security/mcode.h"
#include "security/rp.h"

#include "util/mem.h"

static bool iidxhook_config_boot_seeds_parse(const char *str, uint32_t *seeds)
{
    if (strlen(str) < 5) {
        return false;
    }

    if (str[1] != ':' || str[3] != ':') {
        return false;
    } else {
        seeds[0] = str[0] - '0';
        seeds[1] = str[2] - '0';
        seeds[2] = str[4] - '0';
        return true;
    }
}

static void _iidxhook_util_config_sec_security_mcode_get(const bt_core_config_t *config, const char *path, struct security_mcode *mcode)
{
    char tmp[sizeof(struct security_mcode) + 1];

    bt_core_config_ext_str_get(config, path, tmp, sizeof(tmp));

    if (!security_mcode_parse(tmp, mcode)) {
        log_fatal("Parsing mcode failed: %s", tmp);
    }
}

static void _iidxhook_util_config_sec_boot_seeds_get(const bt_core_config_t *config, uint32_t *boot_seeds)
{
    char tmp[8];

    bt_core_config_ext_str_get(config, "ezusb/security/boot/seed", tmp, sizeof(tmp));

    if (!iidxhook_config_boot_seeds_parse(tmp, boot_seeds)) {
        log_fatal("Parsing security boot seeds failed: %s", tmp);
    }
}

void iidxhook_util_config_sec_get(
    const bt_core_config_t *config,
    iidxhook_config_sec_t *config_sec)
{
    _iidxhook_util_config_sec_security_mcode_get(config, "ezusb/security/boot/version", &config_sec->boot_version);
    _iidxhook_util_config_sec_boot_seeds_get(config, config_sec->boot_seeds);
    _iidxhook_util_config_sec_security_mcode_get(config, "ezusb/security/plug_black_mcode", &config_sec->black_plug_mcode);
}
