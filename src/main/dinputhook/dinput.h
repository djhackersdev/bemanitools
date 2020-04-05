#ifndef DINPUTHOOK_DINPUT_H
#define DINPUTHOOK_DINPUT_H

#include <windows.h>

// This is to stub out the dinput device
// Such that rawinput can still poll keyboards
void dinput_init(void);

#endif
