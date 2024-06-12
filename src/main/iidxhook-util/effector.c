#define LOG_MODULE "effector-hook"

#include <windows.h>

#include "hook/table.h"

#include "iface-core/log.h"

#include "iidxhook-util/effector.h"

#include "util/defs.h"

static BOOL my_EnableEqualizer(int a1);
static BOOL my_GetEqualizerStatus(LPVOID buffer);
static BOOL my_SetEqualizerGain(int a1, int a2);
static BOOL my_SetGlobalEnvironment(int a1);
static BOOL my_SetSpeakerMode(int direct_sound_struct, int a2);

static const struct hook_symbol effector_hook_syms[] = {
    {
        .name = "EnableEqualizer",
        .patch = my_EnableEqualizer,
    },
    {
        .name = "GetEqualizerStatus",
        .patch = my_GetEqualizerStatus,
    },
    {
        .name = "SetEqualizerGain",
        .patch = my_SetEqualizerGain,
    },
    {
        .name = "SetGlobalEnvironment",
        .patch = my_SetGlobalEnvironment,
    },
    {
        .name = "SetSpeakerMode",
        .patch = my_SetSpeakerMode,
    },
};

static BOOL my_EnableEqualizer(int a1)
{
    log_misc("EnableEqualizer");
    // stub
    return TRUE;
}

static BOOL my_GetEqualizerStatus(LPVOID buffer)
{
    log_misc("GetEqualizerStatus");
    // stub
    return TRUE;
}

static BOOL my_SetEqualizerGain(int a1, int a2)
{
    log_misc("SetEqualizerGain");
    // stub
    return TRUE;
}

static BOOL my_SetGlobalEnvironment(int a1)
{
    log_misc("SetGlobalEnvironment");
    // stub
    return TRUE;
}

static BOOL my_SetSpeakerMode(int direct_sound_struct, int a2)
{
    log_misc("SetSpeakerMode");
    // stub
    return TRUE;
}

void effector_hook_init(void)
{
    hook_table_apply(
        NULL, "rteffect.dll", effector_hook_syms, lengthof(effector_hook_syms));

    log_info("Inserted rteffect hooks");
}
