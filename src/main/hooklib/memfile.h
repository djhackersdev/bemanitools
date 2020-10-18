#ifndef HOOKLIB_MEMFILE_H
#define HOOKLIB_MEMFILE_H

#include <stdint.h>
#include <windows.h>

#include "hook/iohook.h"

/**
 * memfile allows you to stub read-only files in memory.
 */

/**
 * This hooks required functions and sets up the hook array.
 * Please remember to install the irp below.
 */
void memfile_hook_init(void);

void memfile_hook_fini(void);

enum memfile_hook_path_mode {
    ABSOLUTE_MATCH = 0x1,
    ENDING_MATCH = 0x2,
};

/**
 * Adds the specified path to the list of files to hook
 *
 * @param path path to hook
 * @param path_mode path matching mode (1: abs, 2: ending)
 * @param data file contents (buffer must exist until fini is called)
 * @param sz size of file
 */
void memfile_hook_add_fd(
    const char *path, enum memfile_hook_path_mode path_mode, const void *data, uint32_t sz);

/**
 * iohook dispatch function. Needs to be installed.
 */
HRESULT memfile_hook_dispatch_irp(struct irp *irp);

#endif
