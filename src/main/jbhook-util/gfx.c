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

static void hook_infodispcore_set_angle(void* this, int angle);

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

// on knit/copious, this gives us right-side-up errors, too!
static const struct hook_symbol jbhook_infodisp_hook_syms[] = {
    {.name = "?set_angle@infodispcore@@QAEXH@Z",
     .patch = hook_infodispcore_set_angle},
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

    hook_table_apply(
        NULL,
        "infodisp.dll",
        jbhook_infodisp_hook_syms,
        lengthof(jbhook_infodisp_hook_syms));

    log_info("Inserted vertical display hooks");
}

static void hook_infodispcore_set_angle(void* this, int angle) {
    // ignore
}

// Welcome to OpenGL land! There is a ton of boilerplate needed here "just" to
// rotate the render output

// only jubeat uses openGL, let alone needs rotation at all, so hardcode for now
#define W 768
#define H 1360

// the framebuffer we redirect renders to instead of rendering to the screen
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

static void __stdcall hook_glFlush(void) {
    // 3 bytes per RGB pixel
    // these could really be stack variables but they are too big for gcc's
    // default stack size, and it's no problem for 6MiB of data to hang around
    static uint8_t pixels_raw[W*H*3];
    static uint8_t pixels_rot[W*H*3];

    glReadPixels(0, 0, H, W, GL_RGB, GL_UNSIGNED_BYTE, pixels_raw);

    // CPU copies may seem slow here, but this runs fine on my jubeat cab, so
    // speed is not a huge concern.
    for(size_t x = 0; x < W; x++) {
        for(size_t y = 0; y < H; y++) {
            memcpy(&pixels_rot[3*(y*W + x)], &pixels_raw[3*((W-x)*H + y)], 3);
        }
    }

    // now *we* get to draw to the main display
    real_glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

    // the scissor test clips pixels to the game window - must be disabled or
    // the draw area is cut off vertically
    glDisable(GL_SCISSOR_TEST);
    glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_BYTE, pixels_rot);

    real_glFlush();

    // reset the framebuffer for the next draw - must be after flush (i.e. when
    // all state is reset) and before the game tries to draw anything
    fb_init();
    real_glBindFramebufferEXT(GL_FRAMEBUFFER, fb);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D,	0, GL_RGBA, H, W, 0, GL_RGBA,GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
}

// hooking something in glhelper.dll (a jubeat supplied DLL) might seem
// insufficiently generic compared to something in opengl32.dll, but the render
// loop of the game makes it very difficult to "catch" the rendering at the
// right place otherwise. This works with all horizontal jubeats, and they
// stopped needing rotation fixes starting with saucer.
static void hook_glBindFramebufferEXT(GLenum target, GLuint framebuffer) {
    fb_init();

    // check this is actually the screen - the game also uses internal
    // framebuffers for some parts of the display
    if(target == GL_FRAMEBUFFER && framebuffer == 0) {
        framebuffer = fb;
    }

    return real_glBindFramebufferEXT(target, framebuffer);
}
