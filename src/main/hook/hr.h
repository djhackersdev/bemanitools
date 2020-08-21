#pragma once

#include <windows.h>

#include <stdint.h>

#define hr_propagate_win32(hr, r) (hr_propagate_win32_(hr), r)

uint32_t hr_to_win32_error(HRESULT hr);
void hr_propagate_win32_(HRESULT hr);
