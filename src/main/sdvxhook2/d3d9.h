#ifndef SDVXHOOK2_D3D9_H
#define SDVXHOOK2_D3D9_H

#include <stdint.h>

/**
 * Hook some d3d9 functions to patch gfx related stuff
 * like enabling window mode.
 */
void d3d9_hook_init(void);

/**
 * Set the game to window mode.
 *
 * @param framed True to add a window frame and make the window
 *        movable, resizable, minizable. False for no frame.
 */
void d3d9_set_windowed(bool framed, int32_t width, int32_t height);

/**
 * Patch the GPU device ID detection to allow running the game with
 * other GPUs than the ones it is locked to.
 *
 * @param vid Vendor ID to patch.
 * @param pid Product ID to patch.
 */
void d3d9_set_pci_id(uint16_t vid, uint16_t pid);

/**
 * Set a framerate limit for the rendering loop.
 *
 * Use this if the game won't sync up properly with vsync enabled.
 *
 * @limit Number of frames to limit the rendering loop to
 */
void d3d9_set_frame_rate_limit(int limit);

#endif
