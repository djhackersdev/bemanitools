#ifndef D3D9EXHOOK_D3D9EX_H
#define D3D9EXHOOK_D3D9EX_H

#include <stdint.h>

#include "d3d9exhook/config-gfx.h"

/**
 * Hook some d3d9 functions to patch gfx related stuff
 * like enabling window mode.
 */
void d3d9ex_hook_init(void);

/**
 * Configure this module by applying the provided config.
 *
 * @param gfx_config Config to apply.
 */
void d3d9ex_configure(struct d3d9exhook_config_gfx *gfx_config);

#endif
