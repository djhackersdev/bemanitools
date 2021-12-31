#define LOG_MODULE "jbhook-gfx"

#include <windows.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "imports/glhelper.h"

#include "jbhook-util/gfx.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/time.h"

static void __stdcall hook_glFlush(void);
static void (__stdcall *real_glFlush)(void);

static void hook_glBindFramebufferEXT(GLenum target, GLuint framebuffer);
static void (*real_glBindFramebufferEXT)(GLenum target, GLuint framebuffer);

static const struct hook_symbol jbhook1_opengl_hook_syms[] = {
    {.name = "glFlush",
     .patch = hook_glFlush,
     .link = (void **) &real_glFlush},
};

static const struct hook_symbol jbhook1_glhelper_hook_syms[] = {
    {.name = "glBindFramebufferEXT",
     .patch = hook_glBindFramebufferEXT,
     .link = (void **) &real_glBindFramebufferEXT},
};

static DWORD STDCALL my_GetGlyphOutline(
    HDC hdc,
    UINT uChar,
    UINT uFormat,
    LPGLYPHMETRICS lpgm,
    DWORD cbBuffer,
    LPVOID lpvBuffer,
    const MAT2 *lpmat2);

static DWORD(STDCALL *real_GetGlyphOutline)(
    HDC hdc,
    UINT uChar,
    UINT uFormat,
    LPGLYPHMETRICS lpgm,
    DWORD cbBuffer,
    LPVOID lpvBuffer,
    const MAT2 *lpmat2);

static const struct hook_symbol gfx_hook_syms[] = {
    {.name = "GetGlyphOutlineA",
     .patch = my_GetGlyphOutline,
     .link = (void **) &real_GetGlyphOutline},
    {
        .name = "IsDBCSLeadByteEx",
        .patch = my_GetGlyphOutline, // !??
        .link = (void **) &real_GetGlyphOutline // !??
    },
};

static DWORD STDCALL my_GetGlyphOutline(
    HDC hdc,
    UINT uChar,
    UINT uFormat,
    LPGLYPHMETRICS lpgm,
    DWORD cbBuffer,
    LPVOID lpvBuffer,
    const MAT2 *lpmat2)
{
    if (IsDBCSLeadByteEx(CP_ACP, uChar & 0xFF)) {
        log_warning("!!!!!!!!");

        char tmp[2];

        tmp[0] = uChar & 0xFF;
        tmp[1] = (uChar >> 8) & 0xFF;

        MultiByteToWideChar(
            CP_ACP, MB_USEGLYPHCHARS, tmp, 2, lpvBuffer, cbBuffer);
    }

    return real_GetGlyphOutline(
        hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}

void jbhook_util_gfx_hook_init(void)
{
    hook_table_apply(NULL, "gdi32.dll", gfx_hook_syms, lengthof(gfx_hook_syms));

    log_info("Inserted gfx hooks");
}

void jbhook_util_gfx_set_windowed(bool framed)
{
}

void jbhook_util_gfx_install_vertical_hooks(void) {
    hook_table_apply(
        NULL,
        "opengl32.dll",
        jbhook1_opengl_hook_syms,
        lengthof(jbhook1_opengl_hook_syms));

    hook_table_apply(
        NULL,
        "glhelper.dll",
        jbhook1_glhelper_hook_syms,
        lengthof(jbhook1_glhelper_hook_syms));

    log_info("Inserted vertical display hooks");
}

// only jubeat uses openGL, let alone needs rotation at all, so hardcode for now
#define W 768
#define H 1360

static GLuint fb;
static GLuint color;

static void fb_init(void) {
    static bool init_done = false;

    if(init_done) {
        return;
    }

    init_done = true;

    glGenFramebuffersEXT(1, &fb);
    glGenTextures(1, &color);
}

static uint8_t pixels_raw[W*H*3];
static uint8_t pixels_rot[W*H*3];

static void __stdcall hook_glFlush(void) {
    glReadPixels(0, 0, H, W, GL_RGB, GL_UNSIGNED_BYTE, pixels_raw);

    for(size_t x = 0; x < W; x++) {
        for(size_t y = 0; y < H; y++) {
            memcpy(&pixels_rot[3*(y*W + x)], &pixels_raw[3*((W-x)*H + y)], 3);
        }
    }

    real_glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

    glDisable(GL_SCISSOR_TEST);
    glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_BYTE, pixels_rot);

    real_glFlush();

    fb_init();
    real_glBindFramebufferEXT(GL_FRAMEBUFFER, fb);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D,	0, GL_RGBA, H, W, 0, GL_RGBA,GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
}

static void hook_glBindFramebufferEXT(GLenum target, GLuint framebuffer) {
    fb_init();

    if(target == GL_FRAMEBUFFER && framebuffer == 0) {
        framebuffer = fb;
    }

    return real_glBindFramebufferEXT(target, framebuffer);
}
