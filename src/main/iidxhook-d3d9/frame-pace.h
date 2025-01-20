#ifndef IIDXHOOK_D3D9_FRAME_PACE_H
#define IIDXHOOK_D3D9_FRAME_PCE_H

#include <stdbool.h>
#include <stdint.h>

#include "hook/d3d9.h"

void iidxhook_d3d9_frame_pace_init(DWORD main_thread_id, double target_frame_rate_hz);

HRESULT iidxhook_d3d9_frame_pace_d3d9_irp_handler(struct hook_d3d9_irp *irp);

#endif