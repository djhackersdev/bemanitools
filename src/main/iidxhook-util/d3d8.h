#ifndef IIDXHOOK_D3D8_H
#define IIDXHOOK_D3D8_H

#include <stdbool.h>

/**
 * Callback function to receive resulting refresh rate of monitor check.
 */
typedef void (*d3d8_monitor_check_result_callback_t)(double refresh_rate);

/**
 * Hook some d3d8 related functions to patch gfx related stuff
 * like enabling window mode.
 *
 * Games using d3d8:
 * - IIDX: 9th to DD
 */
void d3d8_hook_init(void);

/**
 * Set the game to window mode.
 *
 * @param framed True to add a window frame and make the window
 *        movable, resizable, minizable. False for no frame.
 * @param width Width of the window. -1 to keep original width.
 * @param height Height of the window. -1 to keep original height.
 */
void d3d8_set_windowed(bool framed, int32_t width, int32_t height);

/**
 * Set a framerate limit for the rendering loop.
 *
 * Use this if the game won't sync up properly with vsync enabled.
 *
 * @limit Number of frames to limit the rendering loop to
 */
void d3d8_set_frame_rate_limit(float limit);

/**
 * Enable a software a "monitor check" which determines the (avg.) refresh rate
 * of the game. Ensure that your GPU is outputting a stable refresh rate to
 * your monitor. Otherwise, the resulting values fluctuate a lot
 * 
 * @param cb Callback to a function that receives the result of the monitor
 *           check once it completed.
 */
void d3d8_enable_monitor_check(d3d8_monitor_check_result_callback_t cb);

/**
 * Fixes UV coordinates for background videos on
 * IIDX Red to DD. Use this if you experience the game
 * stretching the background videos
 * (see implementation for further details)
 */
void d3d8_iidx_fix_stretched_bg_videos(void);

/**
 * Fixes the 3D background displayed during song select by disabling fog.
 * On some machines/GPUs the background is rendered completely white which
 * is caused by buggy fog rendering. Fog is used to hide the rings appearing
 * but that's quite 'far away' so disabling fog doesn't look bad.
 */
void d3d8_iidx_fix_12_song_select_bg(void);

/**
 * Fixes the 3D background displayed during song select by disabling lighting.
 * On some machines/GPUs the background is rendered completely black which
 * is caused by buggy lighting. Disabling (dynamic) lighting sets the whole
 * scene to ambient lighting which displays everything properly.
 */
void d3d8_iidx_fix_13_song_select_bg(void);

#endif
