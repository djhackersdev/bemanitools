#define LOG_MODULE "core-boot"

#include <debugapi.h>

#include <stdio.h>

#include "core/config-property-node.h"
#include "core/property-mxml.h"
#include "core/property-node-mxml.h"
#include "core/thread-crt.h"

#include "iface-core/log.h"

#include "util/str.h"

#define CORE_BOOT_LOG_MSG_SIZE_MAX 8192

static void _core_boot_log_std_msg(const char *module, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    printf("[%s] ", module);
    vfprintf(stderr, fmt, args);
    printf("\n");

    va_end(args);
}

static void _core_boot_log_debug_msg(const char *module, const char *fmt, ...)
{
    char msg[CORE_BOOT_LOG_MSG_SIZE_MAX];
    char msg2[CORE_BOOT_LOG_MSG_SIZE_MAX];
    va_list args;

    va_start(args, fmt);

    str_vformat(msg, sizeof(msg), fmt, args);

    va_end(args);

    str_format(msg2, sizeof(msg2), "[%s] %s\n", module, msg);
    
    OutputDebugStringA(msg2);
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

    log_misc("Init minimal logging environment done");
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

    log_misc("Init minimal logging environment done");
}

void core_boot(const char *name)
{
    _core_boot_log_std_msg(LOG_MODULE, "Boot: %s", name);

    _core_boot_minimal_logging_std_env_init();

    core_thread_crt_core_api_set();

    core_property_mxml_core_api_set();
    core_property_node_mxml_core_api_set();

    core_config_property_node_core_api_set();
}

void core_boot_dll(const char *name)
{
    _core_boot_log_debug_msg(LOG_MODULE, "Boot: %s", name);

    _core_boot_minimal_logging_debug_env_init();

    core_thread_crt_core_api_set();

    core_property_mxml_core_api_set();
    core_property_node_mxml_core_api_set();

    core_config_property_node_core_api_set();
}