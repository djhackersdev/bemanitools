#ifndef DDRHOOK_P3IO_H
#define DDRHOOK_P3IO_H

#include <windows.h>

#include "hook/iohook.h"

extern const wchar_t p3io_dev_node_prefix[];
extern const wchar_t p3io_dev_node[];

void p3io_ddr_init(void);
void p3io_ddr_fini(void);

#endif
