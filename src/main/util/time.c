#include <windows.h>

#include "iface-core/log.h"

#include "util/time.h"

static uint64_t counter_freq_ns;
static uint64_t counter_freq_us;
static uint64_t counter_freq_ms;

static void time_init_freq()
{
    LARGE_INTEGER ticks_per_sec;
    QueryPerformanceFrequency(&ticks_per_sec);

    counter_freq_ms = ticks_per_sec.QuadPart / 1000;
    counter_freq_us = counter_freq_ms / 1000;
    counter_freq_ns = counter_freq_us / 1000;
}

uint64_t time_get_counter(void)
{
    LARGE_INTEGER time;

    log_assert(QueryPerformanceCounter(&time) == TRUE);

    return (uint64_t) time.QuadPart;
}

uint64_t time_get_elapsed_ns(uint64_t counter_delta)
{
    if (counter_freq_ns == 0) {
        time_init_freq();
    }

    return counter_delta / counter_freq_ns;
}

uint64_t time_get_elapsed_us(uint64_t counter_delta)
{
    if (counter_freq_ns == 0) {
        time_init_freq();
    }

    return counter_delta / counter_freq_us;
}

uint32_t time_get_elapsed_ms(uint64_t counter_delta)
{
    if (counter_freq_ns == 0) {
        time_init_freq();
    }

    return counter_delta / counter_freq_ms;
}