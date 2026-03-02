#ifndef GENINPUT_KBD_NAIVE_H
#define GENINPUT_KBD_NAIVE_H

#include <windows.h>

#include "geninput/hid-mgr.h"

void kbd_naive_init(void);
void kbd_naive_fini(void);
void kbd_naive_event_notify(RAWINPUT *ri);
bool kbd_is_naive(struct hid_stub *stub);

#endif
