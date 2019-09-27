#ifndef GENINPUT_RI_H
#define GENINPUT_RI_H

#include <windows.h>

void ri_init(HWND hwnd);
void ri_scan_devices(void);
void ri_handle_msg(HRAWINPUT msg);
void ri_fini(void);

#endif
