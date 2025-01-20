#define LOG_MODULE "iidxhook-d3d-frame-pace"

#include <windows.h>

#include <stdbool.h>

#include "hook/table.h"

#include "util/log.h"

#include "frame-pace.h"

static void STDCALL my_Sleep(DWORD dwMilliseconds);
static void (STDCALL *real_Sleep)(DWORD dwMilliseconds);

static DWORD STDCALL my_SleepEx(DWORD dwMilliseconds, BOOL bAlertable);
static DWORD (STDCALL *real_SleepEx)(DWORD dwMilliseconds, BOOL bAlertable);

static const struct hook_symbol iidxhok_d3d9_frame_pace_hook_syms[] = {
    {
        .name = "Sleep",
        .patch = my_Sleep,
        .link = (void **) &real_Sleep,
    },
    {
        .name = "SleepEx",
        .patch = my_SleepEx,
        .link = (void **) &real_SleepEx,
    },
};

static bool iidxhook_d3d9_frame_pace_initialized;

static DWORD iidxhook_d3d9_frame_pace_main_thread_id = -1;

static int64_t iidxhook_d3d9_frame_pace_target_frame_time_cpu_ticks;
static int64_t iidxhook_d3d9_frame_pace_frame_time_start_cpu_ticks;

static uint64_t iidxhook_d3d9_frame_pace_get_cpu_tick_frequency()
{
    LARGE_INTEGER freq;

    QueryPerformanceFrequency(&freq);

    return freq.QuadPart;
}

static uint64_t iidxhook_d3d9_frame_pace_get_cpu_ticks()
{
    LARGE_INTEGER tick;

    QueryPerformanceCounter(&tick);

    return tick.QuadPart;
}

// Source and reference implementation:
// https://github.com/PCSX2/pcsx2/blob/f26031cada6893ac306af73255d337e50a8f73f9/pcsx2/Counters.cpp#L563
static void iidxhook_d3d9_frame_pace_do_post_frame()
{
    // -----------------------------------------------------------------------

    const int64_t m_iTicks = iidxhook_d3d9_frame_pace_target_frame_time_cpu_ticks;
    int64_t m_iStart = iidxhook_d3d9_frame_pace_frame_time_start_cpu_ticks;

    // Compute when we would expect this frame to end, assuming everything goes perfectly perfect.
    const uint64_t uExpectedEnd = m_iStart + m_iTicks;
    // The current tick we actually stopped on.
	const uint64_t iEnd = iidxhook_d3d9_frame_pace_get_cpu_ticks();   
    // The diff between when we stopped and when we expected to.
	const int64_t sDeltaTime = iEnd - uExpectedEnd;    

	// If frame ran too long...
	if (sDeltaTime >= m_iTicks)
	{
		// ... Fudge the next frame start over a bit. Prevents fast forward zoomies.
		m_iStart += (sDeltaTime / m_iTicks) * m_iTicks;
        iidxhook_d3d9_frame_pace_frame_time_start_cpu_ticks = m_iStart;
		return;
	}

	// Conversion of delta from CPU ticks (microseconds) to milliseconds
	int32_t msec = (int32_t) ((sDeltaTime * -1000) / (int64_t) iidxhook_d3d9_frame_pace_get_cpu_tick_frequency());

	// If any integer value of milliseconds exists, sleep it off.
	// Prior comments suggested that 1-2 ms sleeps were inaccurate on some OSes;
	// further testing suggests instead that this was utter bullshit.
	if (msec > 1)
	{
		real_Sleep(msec - 1);
	}

	// Conversion to milliseconds loses some precision; after sleeping off whole milliseconds,
	// spin the thread without sleeping until we finally reach our expected end time.
	while (iidxhook_d3d9_frame_pace_get_cpu_ticks() < uExpectedEnd)
	{
		// SKREEEEEEEE
	}

	// Finally, set our next frame start to when this one ends
	m_iStart = uExpectedEnd;
    iidxhook_d3d9_frame_pace_frame_time_start_cpu_ticks = m_iStart;
}

// TODO must be renamed to framerate monitor with smoother/pacer
// TODO have feature flag to print framerate performance counters etc every X seconds
// as misc debug log output
// TODO make sure to record a decent amount of data/frame time accordingly over these
// seconds to report proper avg. frame time/rate, min, max, p95, p99, p999
// TODO move this to a separate module that can be re-used on d3d9ex

// fill up unused frametime on short frames to simulate hardware accuracy
// and match the timing of the target monitor's refresh rate as close as possible
// this fixes frame pacing issues with too short frames not being smoothened
// correctly by the game which either relies entirely on the hardware/GPU driver
// to do that or on tricoro+ era games, on SleepEx which only has max of 1 ms
// accuracy. the further the target monitor refresh rate is away from the desired
// refresh rate, e.g. 60 hz vsync, the more apparent the frame pacing issues
// become in the form of "random stuttering during gameplay"

static void STDCALL my_Sleep(DWORD dwMilliseconds)
{
    // Heuristic, but seems to kill the poorly implemented frame pacing code
    // fairly reliable without impacting other parts of the code negatively
    if (iidxhook_d3d9_frame_pace_main_thread_id == GetCurrentThreadId()) {
        if (dwMilliseconds <= 16) {
            return;
        }
    }

    real_Sleep(dwMilliseconds);
}

static DWORD STDCALL my_SleepEx(DWORD dwMilliseconds, BOOL bAlertable)
{
    // Heuristic, but applies only in two spots
    // - frame pacing code (dynamic value)
    // - Another spot with sleep time set to 1 -> reduces CPU banging
    if (iidxhook_d3d9_frame_pace_main_thread_id == GetCurrentThreadId()) {
        if (dwMilliseconds <= 16) {
            return 0;
        }
    }

    return real_SleepEx(dwMilliseconds, bAlertable);
}

static void iidxhook_d3d9_frame_pace_timings_init(double target_frame_rate_hz)
{
    double tick_rate;

    tick_rate = iidxhook_d3d9_frame_pace_get_cpu_tick_frequency();
    iidxhook_d3d9_frame_pace_target_frame_time_cpu_ticks = (int64_t) (tick_rate / target_frame_rate_hz);
    iidxhook_d3d9_frame_pace_frame_time_start_cpu_ticks = iidxhook_d3d9_frame_pace_get_cpu_ticks();
}

void iidxhook_d3d9_frame_pace_init(DWORD main_thread_id, double target_frame_rate_hz)
{
    log_assert(main_thread_id != -1);

    iidxhook_d3d9_frame_pace_timings_init(target_frame_rate_hz);

    iidxhook_d3d9_frame_pace_main_thread_id = main_thread_id;

    iidxhook_d3d9_frame_pace_initialized = true;

    hook_table_apply(
            NULL, "kernel32.dll", iidxhok_d3d9_frame_pace_hook_syms, lengthof(iidxhok_d3d9_frame_pace_hook_syms));

    log_info("Initialized, target frame rate in hz %f, target frame time in cpu ticks %llu", target_frame_rate_hz, iidxhook_d3d9_frame_pace_target_frame_time_cpu_ticks);
}

HRESULT iidxhook_d3d9_frame_pace_d3d9_irp_handler(struct hook_d3d9_irp *irp)
{
    HRESULT hr;

    log_assert(irp);

    if (!iidxhook_d3d9_frame_pace_initialized) {
        return hook_d3d9_irp_invoke_next(irp);
    }

    if (irp->op == HOOK_D3D9_IRP_OP_DEV_PRESENT) {
        hr = hook_d3d9_irp_invoke_next(irp);

        if (hr == S_OK) {
            iidxhook_d3d9_frame_pace_do_post_frame();
        }

        return hr;
    } else {
        return hook_d3d9_irp_invoke_next(irp);
    }
}