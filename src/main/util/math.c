#include "util/math.h"

int16_t get_wrapped_delta_s16(int16_t val, int16_t last, int16_t bound) {
    int16_t half_point = bound / 2;

    int16_t delta = val - last;

    if (delta > half_point) {
        delta -= bound;
    }

    if (delta < -half_point) {
        delta += bound;
    }

    // delta is now between (-half_point - half_point)
    return delta;
}

int16_t sign_s16(int16_t x) {
    return (x > 0) - (x < 0);
}
