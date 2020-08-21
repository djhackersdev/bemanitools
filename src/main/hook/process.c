#include <windows.h>
#include <tlhelp32.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "hook/hr.h"
#include "hook/pe.h"
#include "hook/process.h"

static bool thread_match_startup(
        const CONTEXT *ctx,
        void *ntstart,
        void *exe_entry)
{
#ifdef _M_AMD64
    return  ctx->Rip == (DWORD64) ntstart &&
            ctx->Rcx == (DWORD64) exe_entry;
#else
    return  ctx->Eip == (DWORD) ntstart &&
            ctx->Eax == (DWORD) exe_entry;
#endif
}

static void thread_patch_startup(
        process_entry_t new_entry,
        process_entry_t *orig_entry,
        CONTEXT *ctx)
{
#ifdef _M_AMD64
    *orig_entry = (void *) ctx->Rcx;
    ctx->Rcx = (DWORD64) new_entry;
#else
    *orig_entry = (void *) ctx->Eax;
    ctx->Eax = (DWORD) new_entry;
#endif
}

static HRESULT process_hijack_try_thread(
        process_entry_t new_entry,
        process_entry_t *orig_entry,
        DWORD thread_id)
{
    CONTEXT ctx;
    HMODULE exe;
    HMODULE ntdll;
    void *exe_entry;
    void *ntstart;
    HANDLE thread;
    HRESULT hr;
    BOOL ok;

    thread = NULL;

    exe = GetModuleHandleW(NULL);

    if (exe == NULL) {
        /* uhhhh... */
        hr = E_UNEXPECTED;

        goto end;
    }

    ntdll = GetModuleHandleW(L"ntdll.dll");

    if (ntdll == NULL) {
        /* Another 2 + 2 = 5 situation */
        hr = E_UNEXPECTED;

        goto end;
    }

    exe_entry = pe_get_entry_point(exe);
    ntstart = GetProcAddress(ntdll, "RtlUserThreadStart");

    if (ntstart == NULL) {
        /* TODO Deal with WinXP, for the poor souls still stuck on that OS.
           XP starts threads at LdrInitializeThunk instead (I think) */
        hr = E_NOTIMPL;

        goto end;
    }

    thread = OpenThread(
            THREAD_GET_CONTEXT | THREAD_SET_CONTEXT,
            FALSE,
            thread_id);

    if (thread == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());

        goto end;
    }

    memset(&ctx, 0, sizeof(ctx));
#ifdef _M_AMD64
    ctx.ContextFlags = CONTEXT_AMD64 | CONTEXT_FULL;
#else
    ctx.ContextFlags = CONTEXT_i386 | CONTEXT_FULL;
#endif

    ok = GetThreadContext(thread, &ctx);

    if (!ok) {
        hr = HRESULT_FROM_WIN32(GetLastError());

        goto end;
    }

    if (thread_match_startup(&ctx, ntstart, exe_entry)) {
        thread_patch_startup(new_entry, orig_entry, &ctx);
        ok = SetThreadContext(thread, &ctx);

        if (!ok) {
            hr = HRESULT_FROM_WIN32(GetLastError());

            goto end;
        }

        return S_OK;
    } else {
        return S_FALSE;
    }

end:
    if (thread != NULL) {
        CloseHandle(thread);
    }

    return hr;
}

HRESULT process_hijack_startup(
        process_entry_t new_entry,
        process_entry_t *orig_entry)
{
    THREADENTRY32 thread;
    HANDLE snap;
    DWORD pid;
    HRESULT fault;
    HRESULT hr;
    BOOL ok;

    assert(new_entry != NULL);
    assert(orig_entry != NULL);

    pid = GetCurrentProcessId();
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (snap == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());

        goto end;
    }

    thread.dwSize = sizeof(thread);
    ok = Thread32First(snap, &thread);

    if (!ok) {
        hr = HRESULT_FROM_WIN32(GetLastError());

        goto end;
    }

    /* Return this if we don't find anything suitable */
    fault = E_FAIL;

    do {
        if (thread.th32OwnerProcessID != pid) {
            continue;
        }

        hr = process_hijack_try_thread(
                new_entry,
                orig_entry,
                thread.th32ThreadID);

        if (hr == S_OK) {
            /* Main thread successfully hijacked, finish up */
            goto end;
        } else if (FAILED(hr)) {
            /* Latch this error code, but don't abort, keep trying. */
            fault = hr;
        }
    } while (Thread32Next(snap, &thread));

    hr = fault;

end:
    if (snap != INVALID_HANDLE_VALUE) {
        CloseHandle(snap);
    }

    return hr;
}
