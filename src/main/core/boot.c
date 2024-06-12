#define LOG_MODULE "core-boot"

#include <debugapi.h>

#include <stdio.h>

#include "iface-core/log.h"

#include "util/str.h"

#define CORE_BOOT_LOG_MODULE_SIZE_MAX 128
/* 64k so we can log data dumps of rs232 without crashing */
#define CORE_BOOT_LOG_MSG_SIZE_MAX 65536

static void _core_boot_log_std_msg(const char *module, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    printf("[%s] ", module);
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
}

static void _core_boot_log_debug_msg(const char *module, const char *fmt, ...)
{
    char module_str[CORE_BOOT_LOG_MODULE_SIZE_MAX];
    char msg[CORE_BOOT_LOG_MSG_SIZE_MAX];
    va_list args;

    va_start(args, fmt);

    str_format(module_str, sizeof(module_str), "[%s] ", module);
    str_vformat(msg, sizeof(msg), fmt, args);
    OutputDebugStringA(module_str);
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");

    va_end(args);
}

static void _core_boot_minimal_logging_std_env_init()
{
    _core_boot_log_std_msg(
        LOG_MODULE, "Init minimal logging environment (std)");

    // Initialize logging directly to avoid a bootstrapping issue
    // with not having any kind of logging to bootstrap the full
    // featured logging system
    _bt_core_log_api.version = 1;

    _bt_core_log_api.v1.misc = _core_boot_log_std_msg;
    _bt_core_log_api.v1.info = _core_boot_log_std_msg;
    _bt_core_log_api.v1.warning = _core_boot_log_std_msg;
    _bt_core_log_api.v1.fatal = _core_boot_log_std_msg;

    log_info("Init minimal logging environment done");
}

static void _core_boot_minimal_logging_debug_env_init()
{
    _core_boot_log_debug_msg(
        LOG_MODULE, "Init minimal logging environment (debug)");

    // Initialize logging directly to avoid a bootstrapping issue
    // with not having any kind of logging to bootstrap the full
    // featured logging system
    _bt_core_log_api.version = 1;

    _bt_core_log_api.v1.misc = _core_boot_log_debug_msg;
    _bt_core_log_api.v1.info = _core_boot_log_debug_msg;
    _bt_core_log_api.v1.warning = _core_boot_log_debug_msg;
    _bt_core_log_api.v1.fatal = _core_boot_log_debug_msg;

    log_info("Init minimal logging environment done");
}

void core_boot(const char *name)
{
    _core_boot_log_std_msg(LOG_MODULE, "Boot: %s", name);

    _core_boot_minimal_logging_std_env_init();
}

void core_boot_dll(const char *name)
{
    _core_boot_log_debug_msg(LOG_MODULE, "Boot: %s", name);

    _core_boot_minimal_logging_debug_env_init();
}