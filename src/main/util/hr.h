#ifndef UTIL_HR_H
#define UTIL_HR_H

#include <windows.h>

#define hr_propagate_win32(hr, r) (hr_propagate_win32_(hr), r)

HRESULT hr_from_win32(void);
void hr_propagate_win32_(HRESULT hr);

#endif
