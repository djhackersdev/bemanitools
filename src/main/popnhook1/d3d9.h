#ifndef POPNHOOK1_D3D9_H
#define POPNHOOK1_D3D9_H

/**
 * Config structure for this module. Enable/disable supported features.
 */
struct popnhook1_d3d9_config {
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
     * Fix an issue with CreateTexture on later versions of Windows that prevents
     * the games from launching.
     */
    bool texture_usage_fix;
};

void popnhook1_d3d9_init(void);
void popnhook1_d3d9_init_config(struct popnhook1_d3d9_config *config);
void popnhook1_d3d9_configure(const struct popnhook1_d3d9_config *config);

#endif
