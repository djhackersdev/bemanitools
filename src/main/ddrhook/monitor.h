#ifndef DDRHOOK_MONITOR_H
#define DDRHOOK_MONITOR_H

#include <windows.h>

DEFINE_GUID(
    monitor_guid,
    0x4D36E96E,
    0xE325,
    0x11CE,
    0xBF,
    0xC1,
    0x08,
    0x00,
    0x2B,
    0xE1,
    0x03,
    0x18);

void monitor_setupapi_insert_hooks(HMODULE target);

#endif
