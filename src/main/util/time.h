#ifndef UTIL_TIME_H
#define UTIL_TIME_H

#include <stdint.h>

uint64_t time_get_counter(void);

uint64_t time_get_elapsed_ns(uint64_t counter_delta);

uint64_t time_get_elapsed_us(uint64_t counter_delta);

uint32_t time_get_elapsed_ms(uint64_t counter_delta);

#endif