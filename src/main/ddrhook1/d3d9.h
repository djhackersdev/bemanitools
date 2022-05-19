#ifndef DDRHOOK1_D3D9_H
#define DDRHOOK1_D3D9_H

#include <stdbool.h>
#include <stdint.h>

#include "hook/d3d9.h"

struct ddrhook1_d3d9_config {
    bool windowed;
};

void ddrhook1_d3d9_hook_init();
void ddrhook1_d3d9_configure(
    const struct ddrhook1_d3d9_config *config);
HRESULT ddrhook1_d3d9_irp_handler(struct hook_d3d9_irp *irp);

#endif
