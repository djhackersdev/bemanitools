#ifndef DDRHOOKX_D3D9_H
#define DDRHOOKX_D3D9_H

#include <stdbool.h>
#include <stdint.h>

#include "hook/d3d9.h"

struct ddrhookx_d3d9_config {
    bool windowed;
};

void ddrhookx_d3d9_hook_init();
void ddrhookx_d3d9_configure(
    const struct ddrhookx_d3d9_config *config);
HRESULT ddrhookx_d3d9_irp_handler(struct hook_d3d9_irp *irp);

#endif
