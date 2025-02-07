#define LOG_MODULE "util-proc"

#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <ntstatus.h>

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"
#include "util/winerr.h"

#include "proc.h"

typedef NTSTATUS (NTAPI *PNtQueryInformationThread)(
    HANDLE ThreadHandle,
    THREADINFOCLASS ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
);

void proc_terminate_current_process(uint32_t exit_code)
{
    HANDLE hnd;

    hnd = OpenProcess(
        SYNCHRONIZE | PROCESS_TERMINATE, TRUE, GetCurrentProcessId());

    TerminateProcess(hnd, exit_code);
}

void* proc_thread_get_proc_address(int thread_id)
{
    HANDLE thread;
    char* err_str;
    HMODULE nt_dll;
    PNtQueryInformationThread ntQueryInformationThread;
    NTSTATUS status;
    PVOID thread_start_address;
    ULONG return_len;

    thread = proc_thread_get_handle(thread_id);

    if (thread == NULL) {
        err_str = util_winerr_format_last_error_code();
        log_fatal("Failed to open thread with id %d: %s", thread_id, err_str);
    }

    nt_dll = LoadLibrary("ntdll.dll");
    ntQueryInformationThread = (PNtQueryInformationThread) GetProcAddress(nt_dll, "NtQueryInformationThread");

    status = ntQueryInformationThread(thread, ThreadQuerySetWin32StartAddress, &thread_start_address, sizeof(thread_start_address), &return_len);
    
    CloseHandle(thread);
    FreeLibrary(nt_dll);

    if (status != STATUS_SUCCESS) {
        log_fatal("Failed to get start address for thread %d: 0x%lx\n", thread_id, status);
    }

    return thread_start_address;
}

HMODULE proc_thread_proc_get_origin_module(void* proc_addr)
{
    HMODULE module;

    log_assert(proc_addr);

    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
               (LPCTSTR) proc_addr, &module)) {
                return NULL;
               } else {
                return module;
               }
}

bool proc_thread_proc_get_origin_module_path(void* proc_addr, char* buffer, size_t len)
{
    HMODULE module;

    module = proc_thread_proc_get_origin_module(proc_addr);

    if (module != NULL) {
        if (GetModuleFileNameA(module, buffer, len) == 0) {
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

bool proc_thread_proc_get_origin_module_name(void* proc_addr, char* buffer, size_t len)
{
    HMODULE module;
    char path_buffer[MAX_PATH];
    const char* filename;

    module = proc_thread_proc_get_origin_module(proc_addr);

    if (module != NULL) {
        if (GetModuleFileNameA(module, path_buffer, sizeof(path_buffer)) == 0) {
            return false;
        } else {
            filename = strrchr(path_buffer, '\\');

            if (filename == NULL) {
                // no backslashes found, use the entire path
                filename = path_buffer;
            } else {
                // skip the backslash
                filename++;
            }

            str_cpy(buffer, len, filename);

            return true;
        }
    } else {
        return false;
    }
}

size_t proc_thread_scan_threads_current_process(struct proc_thread_info** info)
{
    DWORD pid;
    HANDLE snapshot;
    char* err_str;
    THREADENTRY32 thread_entry;
    size_t idx;
    HANDLE thread;

    log_assert(info);
    
    pid = GetCurrentProcessId();

    // Create a snapshot of the running threads in the target process.
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);

    if (snapshot == INVALID_HANDLE_VALUE) {
        err_str = util_winerr_format_last_error_code();
        log_fatal("Failed to create snapshot of threads: %s", err_str);
    }

    thread_entry.dwSize = sizeof(THREADENTRY32);

    idx = 0;
    *info = NULL;

    // Count entries for allocation of return value
    if (Thread32First(snapshot, &thread_entry)) {
        do {
            if (thread_entry.th32OwnerProcessID == pid) {
                *info = xrealloc(*info, (idx + 1) * sizeof(struct proc_thread_info));

                thread = proc_thread_get_handle(thread_entry.th32ThreadID);

                (*info)[idx].id = thread_entry.th32ThreadID;
                (*info)[idx].proc = proc_thread_get_proc_address(thread_entry.th32ThreadID);
                (*info)[idx].priority = GetThreadPriority(thread);
                (*info)[idx].origin_module = proc_thread_proc_get_origin_module((*info)[idx].proc);

                CloseHandle(thread);

                idx++;
            }
        } while (Thread32Next(snapshot, &thread_entry));
    }

    CloseHandle(snapshot);

    return idx;
}

HANDLE proc_thread_get_handle(int thread_id)
{
    HANDLE handle;

    handle = OpenThread(THREAD_ALL_ACCESS, FALSE, thread_id);

    return handle;
}

bool proc_thread_set_priority(int thread_id, int priority)
{
    HANDLE thread;
    BOOL res;

    thread = proc_thread_get_handle(thread_id);

    if (thread == NULL) {
        return false;
    }

    res = SetThreadPriority(thread, priority);

    CloseHandle(thread);

    return res;
}

bool proc_thread_set_affinity(int thread_id, uint32_t cpu_mask)
{
    HANDLE thread;
    DWORD_PTR affinity_mask;
    DWORD_PTR prev_affinity_mask;

    thread = proc_thread_get_handle(thread_id);

    if (thread == NULL) {
        return false;
    }

    affinity_mask = cpu_mask;
    prev_affinity_mask = SetThreadAffinityMask(thread, affinity_mask);
    
    CloseHandle(thread);

    return prev_affinity_mask != 0;
}