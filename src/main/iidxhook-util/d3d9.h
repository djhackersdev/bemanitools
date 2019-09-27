#ifndef IIDXHOOK_D3D9_H
#define IIDXHOOK_D3D9_H

#include <stdint.h>

/**
 * Filter types for scaling back buffer, e.g. upscaling for higher resolution monitors.
 */
enum d3d9_back_buffer_scale_filter {
    D3D9_BACK_BUFFER_SCALE_FILTER_NONE = 0,
    D3D9_BACK_BUFFER_SCALE_FILTER_LINEAR = 1,
    D3D9_BACK_BUFFER_SCALE_FILTER_POINT = 2,
};

/**
 * Callback function to receive resulting refresh rate of monitor check.
 */
typedef void (*d3d9_monitor_check_result_callback_t)(double refresh_rate);

/**
 * Hook some d3d9 functions to patch gfx related stuff
 * like enabling window mode.
 *
 * Games using d3d9:
 * - IIDX: Gold to Copula
 */
void d3d9_hook_init(void);

/**
 * Set the game to window mode.
 *
 * @param framed True to add a window frame and make the window
 *        movable, resizable, minizable. False for no frame.
 * @param width Width of the window. -1 to keep original width.
 * @param height Height of the window. -1 to keep original height.
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
void d3d9_set_frame_rate_limit(float limit);

/**
 * Enable a software a "monitor check" which determines the (avg.) refresh rate
 * of the game. Ensure that your GPU is outputting a stable refresh rate to
 * your monitor. Otherwise, the resulting values fluctuate a lot
 * 
 * @param cb Callback to a function that receives the result of the monitor
 *           check once it completed.
 */
void d3d9_enable_monitor_check(d3d9_monitor_check_result_callback_t cb);

/**
 * Ported from d3d8 module to enable this when using d3d8to9.
 * 
 * Fixes UV coordinates for background videos on
 * IIDX Red to DD. Use this if you experience the game
 * stretching the background videos
 * (see implementation for further details)
 */
void d3d9_iidx_fix_stretched_bg_videos(void);

/**
 * Ported from d3d8 module to enable this when using d3d8to9.
 * 
 * Fixes the 3D background displayed during song select by disabling fog.
 * On some machines/GPUs the background is rendered completely white which
 * is caused by buggy fog rendering. Fog is used to hide the rings appearing
 * but that's quite 'far away' so disabling fog doesn't look bad.
 */
void d3d9_iidx_fix_12_song_select_bg(void);

/**
 * Ported from d3d8 module to enable this when using d3d8to9.
 * 
 * Fixes the 3D background displayed during song select by disabling lighting.
 * On some machines/GPUs the background is rendered completely black which
 * is caused by buggy lighting. Disabling (dynamic) lighting sets the whole
 * scene to ambient lighting which displays everything properly.
 */
void d3d9_iidx_fix_13_song_select_bg(void);

/**
 * Fixes a parameter on CreateTexture calls causing the game to crash with
 * NVIDIA cards (on GOLD and possible newer)
 */
void d3d9_enable_nvidia_fix(void);

/**
 * Fixes UV coordinates for background videos on
 * (similar to the d3d8 games). But, this fixes the quad seam issue on 14 to
 * 17, only.
 */
void d3d9_bg_video_seams_fix(void);

/**
 * Scale the back buffer after a frame got rendered. This allows you to up-/downscale the final frame to display
 * arbitrary resolutions, e.g. 640x480 -> 1920x1080 to allow displaying the game in the monitor's native resolution.
 * This avoids over-/underscan (yes, that's still a thing with todays TVs) or leaving the upscaling to monitors/TVs
 * that are doing a terrible job regarding picture quality or latency wise.
 * 
 * @param width Target width to scale the frame to. 0 disables scaling entirely.
 * @param height Target height to scale the frame to. 0 disables scaling entirely.
 * @param filter Filtering method to use for scaling. Depending on how you scale, this can have an impact on the final
 *               image quality.
 */
void d3d9_scale_back_buffer(uint16_t width, uint16_t height, enum d3d9_back_buffer_scale_filter filter);

#endif
