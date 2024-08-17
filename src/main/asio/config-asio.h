#ifndef ASIOHOOK_CONFIG_IO_H
#define ASIOHOOK_CONFIG_IO_H

#include <windows.h>

#include "cconfig/cconfig.h"

#include "iface-core/config.h"

struct asiohook_config_asio {
    bool force_asio;
    bool force_wasapi;
    char replacement_name[128];
};

void asiohook_config_init(struct cconfig *config);

void asiohook_config_asio_get(
    struct asiohook_config_asio *config_asio, struct cconfig *config);

void asiohook_config_asio_get2(
    const bt_core_config_t *config,
    struct asiohook_config_asio *config_asio);

#endif