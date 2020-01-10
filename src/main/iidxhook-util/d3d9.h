#ifndef IIDXHOOK_D3D9_H
#define IIDXHOOK_D3D9_H

/**
 * This module hooks into various D3D9 calls implementing a large set of
 * features for IIDX games such as: window mode, frame buffer up-/downscaling,
 * software rate limiting, software monitor check,...
 *
 * @author icex2
 */

#include <stdbool.h>
#include <stdint.h>

#include "hook/d3d9.h"

/**
 * Filter types for scaling back buffer, e.g. upscaling for higher resolution
 * monitors.
 */
enum iidxhook_util_d3d9_back_buffer_scale_filter {
    IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_NONE = 0,
    IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_LINEAR = 1,
    IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_POINT = 2,
};

/**
 * Callback function to receive resulting refresh rate of monitor check.
 */
typedef void (*iidxhook_util_d3d9_monitor_check_result_callback_t)(
    double refresh_rate);

/**
 * Config structure for this module. Enable/disable supported features.
 */
struct iidxhook_util_d3d9_config {
    /**
     * Enable/disable window mode. Consider combining with framed.
     */
    bool windowed;

    /**
     * Enable a window frame and make the window movable, resizable, minizable.
     */
    bool framed;

    /**
     * Override the default window height which is determined by the frame
     * buffer size. Set this to 0 to disable the override.
     */
    uint16_t override_window_width;

    /**
     * Override the default window width which is determined by the frame buffer
     * size. Set this to 0 to disable the override.
     */
    uint16_t override_window_height;

    /**
     * Set a software driven framerate limit for the rendering loop and lock to
     * the specified refresh rate, e.g. 59.94. Use this if the game won't sync
     * up properly with vsync enabled.
     */
    float framerate_limit;

    /**
     * Override the GPU vendor ID identifier on display device enumeration.
     * Use this to disable locking to specific GPU cards.
     * Combine with the pci_pid parameter.
     *
     * Vendor ATI: 1002
     */
    uint16_t pci_vid;

    /**
     * Override the GPU product ID identifier on display device enumeration.
     * Use this to disable locking to specific GPU cards.
     * Combine with the pci_pid parameter.
     *
     * PIDs:
     * 7146: RV515      [Radeon X1300]
     * 95C5: RV620 LE   [Radeon HD 3450]
     */
    uint16_t pci_pid;

    /**
     * Enable a software "monitor check" which determines the (avg.) refresh
     * rate of the game. Ensure that your GPU is outputting a stable refresh
     * rate to your monitor. Otherwise, the resulting values fluctuate a lot.
     *
     * Callback to a function that receives the result of the monitor check once
     * it completed.
     */
    iidxhook_util_d3d9_monitor_check_result_callback_t
        iidx09_to_19_monitor_check_cb;

    /**
     * Ported from d3d8 module to enable this when using d3d8to9.
     *
     * Fixes UV coordinates for background videos on IIDX Red to DD and a quad
     * seam issue for 14 to 17. Use this if you experience the game stretching
     * the background videos or seeing a quad seam (see implementation for
     * further details).
     */
    bool iidx11_to_17_fix_uvs_bg_videos;

    /**
     * Ported from d3d8 module to enable this when using d3d8to9.
     *
     * Fixes the 3D background displayed during song select by disabling fog.
     * On some machines/GPUs the background is rendered completely white which
     * is caused by buggy fog rendering. Fog is used to hide the rings appearing
     * but that's quite 'far away' so disabling fog doesn't look bad.
     */
    bool iidx12_fix_song_select_bg;

    /**
     * Ported from d3d8 module to enable this when using d3d8to9.
     *
     * Fixes the 3D background displayed during song select by disabling
     * lighting. On some machines/GPUs the background is rendered completely
     * black which is caused by buggy lighting. Disabling (dynamic) lighting
     * sets the whole scene to ambient lighting which displays everything
     * properly.
     */
    bool iidx13_fix_song_select_bg;

    /**
     * Fixes a parameter on CreateTexture calls causing the game to crash with
     * NVIDIA cards (on GOLD and possible newer)
     */
    bool iidx14_to_19_nvidia_fix;

    /**
     * Scale the back buffer after a frame got rendered. This allows you to
     * up-/downscale the final frame to display arbitrary resolutions, e.g.
     * 640x480 -> 1920x1080 to allow displaying the game in the monitor's native
     * resolution. This avoids over-/underscan (yes, that's still a thing with
     * todays TVs) or leaving the upscaling to monitors/TVs that are doing a
     * terrible job regarding picture quality or latency wise.
     *
     * @param width Target width to scale the frame to. 0 disables scaling
     * entirely.
     * @param height Target height to scale the frame to. 0 disables scaling
     * entirely.
     * @param filter Filtering method to use for scaling. Depending on how you
     * scale, this can have an impact on the final image quality.
     */
    uint16_t scale_back_buffer_width;

    /**
     * Back buffer scale height, see parameter scale_back_buffer_width for full
     * doc.
     */
    uint16_t scale_back_buffer_height;

    /**
     * Back buffer scale filter to apply, see parameter scale_back_buffer_width
     * for full doc.
     */
    enum iidxhook_util_d3d9_back_buffer_scale_filter scale_back_buffer_filter;

    /**
     * Forced refresh rate
     * -1 to not force any (try 59 or 60 if monitor check fails to lock on high
     * refresh rate monitors)
     */
    int32_t forced_refresh_rate;

    /**
     * override device adapter (monitor)
     * 0 (D3DADAPTER_DEFAULT) to use default (whatever the game sends), 1, 2
     * etc. to use specified adapter
     */
    int32_t device_adapter;
};

/**
 * Initialize this hook module. Must be called before any other function of this
 * module.
 *
 * Games using d3d9: IIDX 14 (Gold) to IIDX 27 (Rootage)
 * For d3d8 based games (IIDX 9 to 13): use d3d8to9 wrapper to allow usage of
 * this module.
 */
void iidxhook_util_d3d9_hook_init(void);

/**
 * Initialize the provided config struct with default values.
 *
 * @param config Pointer to an initialized struct to write default values to.
 */
void iidxhook_util_d3d9_init_config(struct iidxhook_util_d3d9_config *config);

/**
 * Configure this module by applying the provided config.
 *
 * @param config Config to apply.
 */
void iidxhook_util_d3d9_configure(
    const struct iidxhook_util_d3d9_config *config);

/**
 * Main IRP handler call for this module. Must be set up with the hook/d3d9
 * module to receive calls from d3d9.
 *
 * @param irp Pointer to a valid IRP.
 * @return S_OK on success, error otherwise (depending on the irp op).
 */
HRESULT iidxhook_util_d3d9_irp_handler(struct hook_d3d9_irp *irp);

#endif
