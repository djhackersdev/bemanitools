#ifndef GENINPUT_HOTPLUG_H
#define GENINPUT_HOTPLUG_H

#include <dbt.h>
#include <windows.h>

void hotplug_init(HWND wnd);
void hotplug_handle_msg(WPARAM wparam, const DEV_BROADCAST_HDR *hdr);
void hotplug_fini(void);

#endif
