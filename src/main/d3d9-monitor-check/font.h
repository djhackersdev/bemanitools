#ifndef D3D9_MONITOR_CHECK_FONT_H
#define D3D9_MONITOR_CHECK_FONT_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"

typedef struct font font_t;

typedef struct font_text {
    font_t *font;
    uint32_t line_spacing;
    uint32_t origin_x;
    uint32_t origin_y;
    uint32_t current_x;
    uint32_t current_y;
} font_text_t;

bool font_init(gfx_t *gfx, uint32_t size, font_t **font);

void font_fini(font_t *font);

void font_text_begin(font_t *font, uint32_t line_spacing, uint32_t origin_x, uint32_t origin_y, font_text_t *text);

void font_text_white_draw(font_text_t *text, const char *fmt, ...);

void font_text_red_draw(font_text_t *text, const char *fmt, ...);

void font_text_cyan_draw(font_text_t *text, const char *fmt, ...);

void font_text_newline(font_text_t *text);

void font_text_end(font_text_t *text);

#endif
