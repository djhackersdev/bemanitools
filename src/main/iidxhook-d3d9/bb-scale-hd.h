#ifndef IIDXHOOK_D3D9_HD_H
#define IIDXHOOK_D3D9_HD_H

#include <stdbool.h>
#include <stdint.h>

#include "hook/d3d9.h"

void iidxhook_d3d9_bb_scale_hd_init(uint16_t width, uint16_t height);

HRESULT iidxhook_d3d9_bb_scale_hd_d3d9_irp_handler(struct hook_d3d9_irp *irp);

#endif