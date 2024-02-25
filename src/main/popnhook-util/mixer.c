#define LOG_MODULE "jbhook-mixer"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include "core/log.h"

#include "hook/table.h"

#include "util/defs.h"

MMRESULT STDCALL hook_mixerGetLineControlsA(
    HMIXEROBJ hmxobj, LPMIXERLINECONTROLSA pmxlc, DWORD fdwControls);
MMRESULT STDCALL
hook_mixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPSA pmxcaps, UINT cbmxcaps);
MMRESULT STDCALL hook_mixerOpen(
    LPHMIXER phmx,
    UINT uMxId,
    DWORD_PTR dwCallback,
    DWORD_PTR dwInstance,
    DWORD fdwOpen);
MMRESULT STDCALL hook_mixerSetControlDetails(
    HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
MMRESULT STDCALL hook_mixerGetControlDetailsA(
    HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
MMRESULT STDCALL hook_mixerClose(HMIXER hmx);
MMRESULT STDCALL
hook_mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINEA pmxl, DWORD fdwInfo);

static const struct hook_symbol mixer_hook_syms[] = {
    {.name = "mixerGetLineControlsA", .patch = hook_mixerGetLineControlsA},
    {.name = "mixerGetDevCapsA", .patch = hook_mixerGetDevCapsA},
    {.name = "mixerOpen", .patch = hook_mixerOpen},
    {.name = "mixerSetControlDetails", .patch = hook_mixerSetControlDetails},
    {.name = "mixerGetControlDetailsA", .patch = hook_mixerGetControlDetailsA},
    {.name = "mixerClose", .patch = hook_mixerClose},
    {.name = "mixerGetLineInfoA", .patch = hook_mixerGetLineInfoA},
};

MMRESULT STDCALL hook_mixerOpen(
    LPHMIXER phmx,
    UINT uMxId,
    DWORD_PTR dwCallback,
    DWORD_PTR dwInstance,
    DWORD fdwOpen)
{
    *(DWORD *) phmx = 1234; // any non-zero value is fine

    return MMSYSERR_NOERROR;
}

MMRESULT STDCALL hook_mixerClose(HMIXER hmx)
{
    return MMSYSERR_NOERROR;
}

MMRESULT STDCALL hook_mixerGetLineControlsA(
    HMIXEROBJ hmxobj, LPMIXERLINECONTROLSA pmxlc, DWORD fdwControls)
{
    return MMSYSERR_NOERROR;
}

MMRESULT STDCALL
hook_mixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPSA pmxcaps, UINT cbmxcaps)
{
    return MMSYSERR_NOERROR;
}

MMRESULT STDCALL hook_mixerGetControlDetailsA(
    HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    return MMSYSERR_NOERROR;
}

MMRESULT STDCALL
hook_mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINEA pmxl, DWORD fdwInfo)
{
    return MMSYSERR_NOERROR;
}

MMRESULT STDCALL hook_mixerSetControlDetails(
    HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    return MMSYSERR_NOERROR;
}

void popnhook_mixer_hook_init(void)
{
    hook_table_apply(
        NULL, "winmm.dll", mixer_hook_syms, lengthof(mixer_hook_syms));

    log_info("Inserted mixer hooks");
}
