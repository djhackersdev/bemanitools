#pragma once

#include <stdint.h>

struct os_version {
    char name[128];
    uint32_t major;
    uint32_t minor;
    uint32_t build;
    uint32_t platform_id;
    char extension[256];
};

/**
 * Get the (real) version information of the Windows OS.
 *
 * @param verison Pointer to version structure. This will be filled in.
 * @return true on success, false on failure.
 */
bool os_version_get(struct os_version *version);

/**
 * Log the version information of the current Windows OS.
 */
void os_version_log();