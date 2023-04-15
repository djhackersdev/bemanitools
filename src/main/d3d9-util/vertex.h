#ifndef D3D9_UTIL_H
#define D3D9_UTIL_H

#include <stdbool.h>
#include <stdint.h>

struct d3d9_util_vertex {
    float x;
    float y;
    float z;
    uint32_t color;
    uint32_t unknown;
    float tu;
    float tv;
};

inline bool
d3d9_util_vertex_in_range(float v, float lower_bound_inc, float upper_bound_inc)
{
    return v >= lower_bound_inc && v <= upper_bound_inc;
}

#endif