#ifndef JBHOOK_D3D8_H
#define JBHOOK_D3D8_H

#include <stdbool.h>

/**
 * Hook some gfx related functions to patch gfx related stuff
 * like enabling window mode.
 */
void jbhook_util_gfx_hook_init(void);

/**
 * Set the game to window mode.
 *
 * @param framed True to add a window frame and make the window
 *        movable, resizable, minizable. False for no frame.
 */
void jbhook_util_gfx_set_windowed(bool framed);

/**
 * Rotate a horizontal-by-default jubeat window to vertical
 */
void jbhook_util_gfx_install_vertical_hooks(void);

#endif
