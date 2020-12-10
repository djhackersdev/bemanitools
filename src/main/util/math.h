#ifndef UTIL_MATH_H
#define UTIL_MATH_H

#include <stdint.h>

int16_t get_wrapped_delta_s16(int16_t val, int16_t last, int16_t bound);
int16_t sign_s16(int16_t x);

#endif
