#include <windows.h>

#include <d3dx9.h>
#include <d3dx9core.h>

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "d3d9-monitor-check/font.h"
#include "d3d9-monitor-check/gfx.h"

#include "util/mem.h"

#define BASE_RESOLUTION_WIDTH 640.0f
#define BASE_RESOLUTION_HEIGHT 480.0f
#define BASE_FONT_SIZE 20.0f
#define BASE_MARGIN 1.0f
// Standard Windows DPI
#define BASE_DPI 96.0f

typedef struct font {
    gfx_t *gfx;
    ID3DXFont *font;
    float scale_x;
    float scale_y;
    float dpi_scale;
    uint32_t font_height;
    uint32_t line_height;
    uint32_t base_offset_x;
    uint32_t base_offset_y;
} font_t;

static void _font_text_draw(font_t *font, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, const char *text)
{
    uint32_t scaled_x;
    uint32_t scaled_y;
    RECT rect;

    assert(font);
    assert(text);

    // Scale position based on both resolution and DPI
    scaled_x = (uint32_t)(x * font->scale_x * font->dpi_scale) + font->base_offset_x;
    scaled_y = (uint32_t)(y * font->scale_y * font->dpi_scale) + font->base_offset_y;

    rect.left = scaled_x;
    rect.top = scaled_y;
    // Scale text box width based on both resolution and DPI
    rect.right = scaled_x + (uint32_t)(BASE_RESOLUTION_WIDTH * font->scale_x * font->dpi_scale);
    rect.bottom = scaled_y + font->line_height;

    ID3DXFont_DrawText(
        font->font,
        NULL,
        text,
        -1,
        &rect,
        DT_LEFT | DT_TOP | DT_NOCLIP,
        D3DCOLOR_XRGB(r, g, b));
}

bool font_init(gfx_t *gfx, uint32_t size, font_t **font)
{
    IDirect3DDevice9 *device;
    HRESULT hr;
    HDC hdc;
    int dpi;

    assert(gfx);
    assert(font);

    *font = xmalloc(sizeof(font_t));
    memset(*font, 0, sizeof(font_t));

    (*font)->gfx = gfx;

    // Get the system DPI
    hdc = GetDC(NULL);
    dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);

    // Calculate DPI scale relative to base DPI
    (*font)->dpi_scale = dpi / BASE_DPI;

    // Calculate resolution scaling factors
    (*font)->scale_x = gfx_width_get(gfx) / BASE_RESOLUTION_WIDTH;
    (*font)->scale_y = gfx_height_get(gfx) / BASE_RESOLUTION_HEIGHT;
    
    // Scale font height based on both DPI and resolution
    // This ensures text maintains physical size across different resolutions
    (*font)->font_height = (uint32_t)(size * (*font)->dpi_scale);
    
    // Line height matches font height
    (*font)->line_height = (*font)->font_height;
    
    // Scale margins based on DPI and resolution
    (*font)->base_offset_x = (uint32_t)(BASE_MARGIN * (*font)->scale_x * (*font)->dpi_scale);
    (*font)->base_offset_y = (uint32_t)(BASE_MARGIN * (*font)->scale_y * (*font)->dpi_scale);

    device = gfx_device_get(gfx);

    hr = D3DXCreateFont(
        device,
        (*font)->font_height,
        0,
        FW_BOLD,
        1,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,  // Changed to antialiased for better readability
        DEFAULT_PITCH | FF_DONTCARE,
        "Arial",
        &(*font)->font);

    if (hr != D3D_OK) {
        free(*font);
        return false;
    }

    return true;
}

void font_fini(font_t *font)
{
    assert(font);

    ID3DXFont_Release(font->font);

    free(font);
}

void font_text_begin(font_t *font, uint32_t line_spacing, uint32_t origin_x, uint32_t origin_y, font_text_t *text)
{
    assert(font);
    assert(text);

    text->font = font;
    text->line_spacing = line_spacing;
    text->origin_x = origin_x;
    text->origin_y = origin_y;
    text->current_x = origin_x;
    text->current_y = origin_y;
}

void font_text_white_draw(font_text_t *text, const char *fmt, ...)
{
    va_list args;
    char buffer[4096];

    assert(text);
    assert(fmt);

    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    _font_text_draw(text->font, text->current_x, text->current_y, 255, 255, 255, buffer);
}

void font_text_red_draw(font_text_t *text, const char *fmt, ...)
{
    va_list args;
    char buffer[4096];

    assert(text);
    assert(fmt);

    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    _font_text_draw(text->font, text->current_x, text->current_y, 255, 0, 0, buffer);
}

void font_text_cyan_draw(font_text_t *text, const char *fmt, ...)
{
    va_list args;
    char buffer[4096];

    assert(text);
    assert(fmt);
    
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    _font_text_draw(text->font, text->current_x, text->current_y, 0, 255, 255, buffer);
}

void font_text_newline(font_text_t *text)
{
    assert(text);

    text->current_x = text->origin_x;
    text->current_y += text->font->line_height + text->line_spacing;
}

void font_text_end(font_text_t *text)
{
    assert(text);

    memset(text, 0, sizeof(font_text_t));
}