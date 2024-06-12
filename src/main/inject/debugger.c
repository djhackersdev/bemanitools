#define LOG_MODULE "inject-debugger"

#include <windows.h>

#include <psapi.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/log-bt.h"

#include "iface-core/log.h"

#include "inject/debugger.h"

#include "util/debug.h"
#include "util/mem.h"
#include "util/proc.h"
#include "util/str.h"

#define MM_ALLOCATION_GRANULARITY 0x10000

struct debugger_thread_params {
    const char *app_name;
    char *cmd_line;
    bool local_debugger;
};

static HANDLE debugger_thread_handle;
static HANDLE debugger_ready_event;

static PROCESS_INFORMATION pi;

static PVOID load_nt_header_from_process(
    HANDLE hProcess, HMODULE hModule, PIMAGE_NT_HEADERS32 pNtHeader);

static HMODULE enumerate_modules_in_process(
    HANDLE hProcess, HMODULE hModuleLast, PIMAGE_NT_HEADERS32 pNtHeader);

// Source:
// https://docs.microsoft.com/en-us/windows/win32/memory/obtaining-a-file-name-from-a-file-handle
static bool
debugger_get_file_name_from_handle(HANDLE hFile, char *filename, size_t bufsize)
{
    HANDLE file_map;
    bool success;

    success = false;

    // Create a file mapping object.
    file_map = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 1, NULL);

    if (file_map) {
        // Create a file mapping to get the file name.
        void *mem = MapViewOfFile(file_map, FILE_MAP_READ, 0, 0, 1);

        if (mem) {
            if (GetMappedFileNameA(
                    GetCurrentProcess(), mem, filename, MAX_PATH)) {

                // Translate path with device name to drive letters.
                char tmp[bufsize];
                tmp[0] = '\0';

                if (GetLogicalDriveStringsA(bufsize - 1, tmp)) {
                    char name[MAX_PATH];
                    char drive[3] = " :";
                    BOOL found = FALSE;
                    char *p = tmp;

                    do {
                        // Copy the drive letter to the template string
                        *drive = *p;

                        // Look up each device name
                        if (QueryDosDevice(drive, name, MAX_PATH)) {
                            size_t name_len = strlen(name);

                            if (name_len < MAX_PATH) {
                                found =
                                    strncmp(filename, name, name_len) == 0 &&
                                    *(filename + name_len) == '\\';

                                if (found) {
                                    // Reconstruct filename using tmp_file
                                    // Replace device path with DOS path
                                    char tmp_file[MAX_PATH];
                                    sprintf(
                                        tmp_file,
                                        "%s%s",
                                        drive,
                                        filename + name_len);

                                    strcpy(filename, tmp_file);
                                }
                            }
                        }

                        // Go to the next NULL character.
                        while (*p++)
                            ;
                    } while (!found && *p); // end of string
                }
            }
            success = true;
            UnmapViewOfFile(mem);
        }

        CloseHandle(file_map);
    }

    return success;
}

static char *
read_debug_str(HANDLE process, const OUTPUT_DEBUG_STRING_INFO *odsi)
{
    log_assert(process);
    log_assert(odsi);

    char *str;

    str = xmalloc(odsi->nDebugStringLength);

    if (ReadProcessMemory(
            process,
            odsi->lpDebugStringData,
            str,
            odsi->nDebugStringLength,
            NULL)) {
        str[odsi->nDebugStringLength - 1] = '\0';
    } else {
        free(str);

        log_warning(
            "ERROR: ReadProcessMemory for debug string failed: %08x",
            (unsigned int) GetLastError());
        str = NULL;
    }

    return str;
}

static char *
read_debug_wstr(HANDLE process, const OUTPUT_DEBUG_STRING_INFO *odsi)
{
    log_assert(process);
    log_assert(odsi);

    char *str;
    wchar_t *wstr;
    uint32_t nbytes;

    nbytes = odsi->nDebugStringLength * sizeof(wchar_t);
    wstr = xmalloc(nbytes);

    if (ReadProcessMemory(
            process, odsi->lpDebugStringData, wstr, nbytes, NULL)) {
        if (wstr_narrow(wstr, &str)) {
            str[odsi->nDebugStringLength - 1] = '\0';
        } else {
            log_warning("ERROR: OutputDebugStringW: UTF-16 conversion failed");
            str = NULL;
        }
    } else {
        log_warning(
            "ERROR: ReadProcessMemory for debug string failed: %08x",
            (unsigned int) GetLastError());
        str = NULL;
    }

    free(wstr);

    return str;
}

static bool log_debug_str(HANDLE process, const OUTPUT_DEBUG_STRING_INFO *odsi)
{
    log_assert(odsi);

    char *debug_str;
    size_t debug_str_len;

    if (odsi->fUnicode) {
        debug_str = read_debug_wstr(process, odsi);
    } else {
        debug_str = read_debug_str(process, odsi);
    }

    if (debug_str) {
        debug_str_len = strlen(debug_str);

        core_log_bt_direct_sink_write(debug_str, debug_str_len);

        free(debug_str);
        return true;
    } else {
        return false;
    }
}

static bool debugger_create_process(
    bool local_debugger, const char *app_name, char *cmd_line)
{
    log_assert(app_name);
    log_assert(cmd_line);

    STARTUPINFO si;
    BOOL ok;
    DWORD flags;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    flags = 0;

    // CREATE_SUSPENDED that we have plenty of time to set up the debugger and
    // theemote process environment with hook dlls.
    flags |= CREATE_SUSPENDED;

    if (local_debugger) {
        // DEBUG_PROCESS is required to make this work properly. Otherwise,
        // weird things like random remote process crashing are happening. Also,
        // DEBUG_ONLY_THIS_PROCESS is NOT sufficient/ correct here. Maybe I
        // didn't understand the documentation properly or it and various blog
        // posts I read are not explaining things well enough or are even wrong.
        flags |= DEBUG_PROCESS;

        log_info("Local debugger enabled");
    } else {
        log_info("Local debugger disabled, no log output from remote process");
    }

    log_misc("Creating remote process %s...", app_name);
    log_misc("Remote process cmd_line: %s", cmd_line);

    ok = CreateProcess(
        app_name, cmd_line, NULL, NULL, FALSE, flags, NULL, NULL, &si, &pi);

    if (!ok) {
        log_warning(
            "ERROR: Failed to launch hooked EXE: %08x",
            (unsigned int) GetLastError());

        free(cmd_line);

        return false;
    }

    free(cmd_line);

    log_info("Remote process created, pid: %ld", pi.dwProcessId);

    return true;
}

static uint32_t debugger_loop()
{
    DEBUG_EVENT de;
    DWORD continue_status;
    char str_buffer[MAX_PATH + 1];

    memset(str_buffer, 0, sizeof(str_buffer));

    for (;;) {
        if (!WaitForDebugEvent(&de, INFINITE)) {
            log_warning(
                "ERROR: WaitForDebugEvent failed: %08x",
                (unsigned int) GetLastError());
            return 1;
        }

        // Changed if applicable, e.g. on exceptions
        continue_status = DBG_CONTINUE;

        switch (de.dwDebugEventCode) {
            case EXCEPTION_DEBUG_EVENT:
                log_misc(
                    "EXCEPTION_DEBUG_EVENT(pid %ld, tid %ld): x%s 0x%p",
                    de.dwProcessId,
                    de.dwThreadId,
                    debug_exception_code_to_str(
                        de.u.Exception.ExceptionRecord.ExceptionCode),
                    de.u.Exception.ExceptionRecord.ExceptionAddress);

                if (de.u.Exception.ExceptionRecord.ExceptionCode ==
                    EXCEPTION_BREAKPOINT) {
                    // Handle breakpoints by ignoring them. Some modules of some
                    // games set breakpoints when they detect a debugger likely
                    // used for actual development/debugging tasks
                    continue_status = DBG_EXCEPTION_HANDLED;
                } else {
                    continue_status = DBG_EXCEPTION_NOT_HANDLED;
                }

                break;

            case CREATE_THREAD_DEBUG_EVENT:
                log_misc(
                    "CREATE_THREAD_DEBUG_EVENT(pid %ld, tid %ld): hnd 0x%p, "
                    "addr 0x%p",
                    de.dwProcessId,
                    de.dwThreadId,
                    de.u.CreateThread.hThread,
                    de.u.CreateThread.lpStartAddress);
                break;

            case CREATE_PROCESS_DEBUG_EVENT:
                GetProcessImageFileNameA(
                    de.u.CreateProcessInfo.hProcess,
                    str_buffer,
                    sizeof(str_buffer));

                log_misc(
                    "CREATE_PROCESS_DEBUG_EVENT(pid %ld, tid %ld): name %s, "
                    "pid %ld",
                    de.dwProcessId,
                    de.dwThreadId,
                    str_buffer,
                    GetProcessId(de.u.CreateProcessInfo.hProcess));

                CloseHandle(de.u.CreateProcessInfo.hFile);

                break;

            case EXIT_THREAD_DEBUG_EVENT:
                log_misc(
                    "EXIT_THREAD_DEBUG_EVENT(pid %ld, tid %ld)",
                    de.dwProcessId,
                    de.dwThreadId);
                break;

            case EXIT_PROCESS_DEBUG_EVENT:
                log_misc(
                    "EXIT_PROCESS_DEBUG_EVENT(pid %ld, tid %ld)",
                    de.dwProcessId,
                    de.dwThreadId);
                return 0;

            case LOAD_DLL_DEBUG_EVENT:
                if (!debugger_get_file_name_from_handle(
                        de.u.LoadDll.hFile, str_buffer, sizeof(str_buffer))) {
                    strcpy(str_buffer, "--- Unknown ---");
                }

                log_misc(
                    "LOAD_DLL_DEBUG_EVENT(pid %ld, tid %ld): name %s, base "
                    "0x%p",
                    de.dwProcessId,
                    de.dwThreadId,
                    str_buffer,
                    de.u.LoadDll.lpBaseOfDll);

                CloseHandle(de.u.LoadDll.hFile);

                break;

            case UNLOAD_DLL_DEBUG_EVENT:
                log_misc(
                    "UNLOAD_DLL_DEBUG_EVENT(pid %ld, tid %ld): base 0x%p",
                    de.dwProcessId,
                    de.dwThreadId,
                    de.u.UnloadDll.lpBaseOfDll);

                break;

            case OUTPUT_DEBUG_STRING_EVENT:
                log_debug_str(pi.hProcess, &de.u.DebugString);

                break;

            case RIP_EVENT:
                log_misc(
                    "RIP_EVENT(pid %ld, tid %ld)",
                    de.dwProcessId,
                    de.dwThreadId);
                break;

            default:
                // Ignore other events
                break;
        }

        if (!ContinueDebugEvent(
                de.dwProcessId, de.dwThreadId, continue_status)) {
            log_warning(
                "ERROR: ContinueDebugEvent failed: %08x",
                (unsigned int) GetLastError());
            return 1;
        }
    }
}

static DWORD WINAPI debugger_proc(LPVOID param)
{
    uint32_t debugger_loop_exit_code;

    struct debugger_thread_params *params;

    params = (struct debugger_thread_params *) param;

    log_misc(
        "Debugger thread start (local debugger: %d)", params->local_debugger);

    if (!debugger_create_process(
            params->local_debugger, params->app_name, params->cmd_line)) {
        return 0;
    }

    SetEvent(debugger_ready_event);

    // Don't run our local debugger loop if the user wants to attach a remote
    // debugger or debugger is disabled
    if (params->local_debugger) {
        debugger_loop_exit_code = debugger_loop();

        free(params);

        log_misc("Debugger loop quit");
        log_info("Terminating process, exit code: %d", debugger_loop_exit_code);

        proc_terminate_current_process(debugger_loop_exit_code);

        // Never reached
        return 0;
    } else {
        free(params);

        log_misc("Debugger thread end");

        return 0;
    }
}

bool debugger_init(bool local_debugger, const char *app_name, char *cmd_line)
{
    struct debugger_thread_params *thread_params;

    debugger_ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (!debugger_ready_event) {
        free(cmd_line);

        log_warning(
            "ERROR: Creating event object failed: %08x",
            (unsigned int) GetLastError());
        return false;
    }

    // free'd by thread if created successfully
    thread_params = xmalloc(sizeof(struct debugger_thread_params));

    thread_params->app_name = app_name;
    thread_params->cmd_line = cmd_line;
    thread_params->local_debugger = local_debugger;

    debugger_thread_handle =
        CreateThread(NULL, 0, debugger_proc, thread_params, 0, 0);

    if (!debugger_thread_handle) {
        free(cmd_line);
        free(thread_params);

        log_warning(
            "ERROR: Creating debugger thread failed: %08x",
            (unsigned int) GetLastError());
        return false;
    }

    WaitForSingleObject(debugger_ready_event, INFINITE);

    log_misc("Debugger initialized");

    return true;
}

bool debugger_wait_for_remote_debugger()
{
    BOOL res;

    log_warning("Waiting until remote debugger attaches to remote process...");

    while (true) {
        res = FALSE;

        if (!CheckRemoteDebuggerPresent(pi.hProcess, &res)) {
            log_warning(
                "ERROR: CheckRemoteDebuggerPresent failed: %08x",
                (unsigned int) GetLastError());
            return false;
        }

        if (res) {
            log_info("Remote debugger attached, resuming");
            break;
        }

        Sleep(1000);
    }

    return true;
}

bool debugger_inject_dll(const char *path_dll)
{
    log_assert(path_dll);

    char dll_path[MAX_PATH];
    DWORD dll_path_length;
    void *remote_addr;
    BOOL ok;
    HANDLE remote_thread;

    log_misc("Injecting: %s", path_dll);

    dll_path_length =
        SearchPath(NULL, path_dll, NULL, MAX_PATH, dll_path, NULL);

    dll_path_length++;

    remote_addr = VirtualAllocEx(
        pi.hProcess,
        NULL,
        dll_path_length,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE);

    if (!remote_addr) {
        log_warning(
            "ERROR: VirtualAllocEx failed: %08x",
            (unsigned int) GetLastError());

        goto alloc_fail;
    }

    ok = WriteProcessMemory(
        pi.hProcess, remote_addr, dll_path, dll_path_length, NULL);

    if (!ok) {
        log_warning(
            "ERROR: WriteProcessMemory failed: %08x",
            (unsigned int) GetLastError());

        goto write_fail;
    }

    remote_thread = CreateRemoteThread(
        pi.hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) LoadLibrary,
        remote_addr,
        0,
        NULL);

    if (remote_thread == NULL) {
        log_warning(
            "ERROR: CreateRemoteThread failed: %08x",
            (unsigned int) GetLastError());

        goto inject_fail;
    }

    WaitForSingleObject(remote_thread, INFINITE);
    CloseHandle(remote_thread);

    ok = VirtualFreeEx(pi.hProcess, remote_addr, 0, MEM_RELEASE);
    remote_addr = NULL;

    if (!ok) {
        log_warning(
            "ERROR: VirtualFreeEx failed: %08x", (unsigned int) GetLastError());
    }

    return true;

inject_fail:
write_fail:
    if (remote_addr != NULL) {
        VirtualFreeEx(pi.hProcess, remote_addr, 0, MEM_RELEASE);
    }

alloc_fail:
    return false;
}

HRESULT debugger_pe_patch_remote(
    HANDLE hProcess, void *dest, const void *src, size_t nbytes)
{
    DWORD old_protect;
    BOOL ok;

    log_assert(dest != NULL);
    log_assert(src != NULL);

    ok = VirtualProtectEx(
        hProcess, dest, nbytes, PAGE_EXECUTE_READWRITE, &old_protect);

    if (!ok) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    ok = WriteProcessMemory(hProcess, dest, src, nbytes, NULL);

    if (!ok) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    ok = VirtualProtectEx(hProcess, dest, nbytes, old_protect, &old_protect);

    if (!ok) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

bool debugger_replace_dll_iat(
    const char *expected_dll, const char *replacement_path_dll)
{
    log_assert(expected_dll);
    log_assert(replacement_path_dll);

    HMODULE hModule = NULL;
    HMODULE hLast = NULL;
    void *remote_addr;

    // Find EXE base in process memory
    IMAGE_NT_HEADERS inh;
    for (;;) {
        memset(&inh, 0, sizeof(IMAGE_NT_HEADERS));

        if ((hLast = enumerate_modules_in_process(pi.hProcess, hLast, &inh)) ==
            NULL) {
            break;
        }

        if ((inh.FileHeader.Characteristics & IMAGE_FILE_DLL) == 0) {
            hModule = hLast;
            break;
        }
    }

    if (hModule == NULL) {
        log_warning("Couldn't find target EXE for hooking");
        goto inject_fail;
    }

    // Search through import table if it exists and replace the target DLL with
    // our DLL filename
    PBYTE pbModule = (PBYTE) hModule;
    PIMAGE_SECTION_HEADER pRemoteSectionHeaders =
        (PIMAGE_SECTION_HEADER) ((PBYTE) pbModule + sizeof(inh.Signature) +
                                 sizeof(inh.FileHeader) +
                                 inh.FileHeader.SizeOfOptionalHeader);
    size_t total_size = inh.OptionalHeader.SizeOfHeaders;

    IMAGE_SECTION_HEADER header;
    for (DWORD n = 0; n < inh.FileHeader.NumberOfSections; ++n) {
        if (!ReadProcessMemory(
                pi.hProcess,
                pRemoteSectionHeaders + n,
                &header,
                sizeof(header),
                NULL)) {
            log_warning("Couldn't read section header: %lu", GetLastError());
            goto inject_fail;
        }

        size_t new_total_size = header.VirtualAddress + header.Misc.VirtualSize;
        if (new_total_size > total_size)
            total_size = new_total_size;
    }

    remote_addr = VirtualAllocEx(
        pi.hProcess,
        pbModule + total_size + MM_ALLOCATION_GRANULARITY,
        strlen(replacement_path_dll) + 1,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE);

    log_assert(remote_addr != NULL);

    debugger_pe_patch_remote(
        pi.hProcess,
        remote_addr,
        replacement_path_dll,
        strlen(replacement_path_dll));

    if (inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
            .VirtualAddress != 0) {
        PIMAGE_IMPORT_DESCRIPTOR pImageImport =
            (PIMAGE_IMPORT_DESCRIPTOR) (pbModule +
                                        inh.OptionalHeader
                                            .DataDirectory
                                                [IMAGE_DIRECTORY_ENTRY_IMPORT]
                                            .VirtualAddress);

        DWORD size = 0;
        while (inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
                       .Size == 0 ||
               size < inh.OptionalHeader
                          .DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
                          .Size) {
            IMAGE_IMPORT_DESCRIPTOR ImageImport;
            if (!ReadProcessMemory(
                    pi.hProcess,
                    pImageImport,
                    &ImageImport,
                    sizeof(ImageImport),
                    NULL)) {
                log_warning("Couldn't read import: %lu", GetLastError());
                goto inject_fail;
            }

            if (ImageImport.Name == 0) {
                break;
            }

            char name[MAX_PATH] = {0};
            if (ReadProcessMemory(
                    pi.hProcess,
                    pbModule + ImageImport.Name,
                    name,
                    sizeof(name),
                    NULL)) {
                // log_misc("\tImport DLL: %ld %s", ImageImport.Name, name);

                if (strcmp(name, expected_dll) == 0) {
                    // log_misc("Replacing %s with %s", name,
                    // replacement_path_dll, (void*)pImageImport);

                    ImageImport.Name = (DWORD) ((PBYTE) remote_addr - pbModule);

                    debugger_pe_patch_remote(
                        pi.hProcess,
                        pImageImport,
                        &ImageImport,
                        sizeof(ImageImport));
                }
            }

            pImageImport++;
            size += sizeof(IMAGE_IMPORT_DESCRIPTOR);
        }
    } else {
        log_warning("Couldn't find import table, can't hook DLL\n");
        goto inject_fail;
    }

    return true;

inject_fail:
    return false;
}

bool debugger_resume_process()
{
    log_info("Resuming remote process...");

    if (ResumeThread(pi.hThread) == -1) {
        log_warning(
            "ERROR: Resuming remote process: %08x",
            (unsigned int) GetLastError());
        return false;
    }

    CloseHandle(pi.hThread);

    return true;
}

void debugger_wait_process_exit()
{
    log_misc("Waiting for remote process to exit...");

    // Wait for the process as we might have a remote debugger attached, so our
    // debugger thread exits after creating the process
    WaitForSingleObject(pi.hProcess, INFINITE);

    // When the process exits, the debugger gets notified and the thread ends
    WaitForSingleObject(debugger_thread_handle, INFINITE);

    log_misc("Remote process exit'd");
}

void debugger_finit(bool failure)
{
    log_misc("Debugger finit");

    if (failure) {
        TerminateProcess(pi.hProcess, EXIT_FAILURE);
        WaitForSingleObject(debugger_thread_handle, INFINITE);
    }

    CloseHandle(debugger_thread_handle);
    CloseHandle(debugger_ready_event);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

// Helper functions based on Microsoft Detours
static PVOID load_nt_header_from_process(
    HANDLE hProcess, HMODULE hModule, PIMAGE_NT_HEADERS32 pNtHeader)
{
    PBYTE pbModule = (PBYTE) hModule;

    memset(pNtHeader, 0, sizeof(*pNtHeader));

    if (pbModule == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    MEMORY_BASIC_INFORMATION mbi;
    memset(&mbi, 0, sizeof(mbi));

    if (VirtualQueryEx(hProcess, hModule, &mbi, sizeof(mbi)) == 0) {
        return NULL;
    }

    IMAGE_DOS_HEADER idh;
    if (!ReadProcessMemory(hProcess, pbModule, &idh, sizeof(idh), NULL)) {
        log_warning("Could not read DOS header: %lu", GetLastError());
        return NULL;
    }

    if (idh.e_magic != IMAGE_DOS_SIGNATURE ||
        (DWORD) idh.e_lfanew > mbi.RegionSize ||
        (DWORD) idh.e_lfanew < sizeof(idh)) {
        return NULL;
    }

    if (!ReadProcessMemory(
            hProcess,
            pbModule + idh.e_lfanew,
            pNtHeader,
            sizeof(*pNtHeader),
            NULL)) {
        log_warning("Could not read NT header: %lu", GetLastError());
        return NULL;
    }

    if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
        return NULL;
    }

    return pbModule + idh.e_lfanew;
}

static HMODULE enumerate_modules_in_process(
    HANDLE hProcess, HMODULE hModuleLast, PIMAGE_NT_HEADERS32 pNtHeader)
{
    PBYTE pbLast = (PBYTE) hModuleLast + MM_ALLOCATION_GRANULARITY;

    memset(pNtHeader, 0, sizeof(*pNtHeader));

    MEMORY_BASIC_INFORMATION mbi;
    memset(&mbi, 0, sizeof(mbi));

    // Find the next memory region that contains a mapped PE image.
    for (;; pbLast = (PBYTE) mbi.BaseAddress + mbi.RegionSize) {
        if (VirtualQueryEx(hProcess, (PVOID) pbLast, &mbi, sizeof(mbi)) == 0) {
            break;
        }

        // Usermode address space has such an unaligned region size always at
        // the end and only at the end.
        if ((mbi.RegionSize & 0xfff) == 0xfff) {
            break;
        }
        if (((PBYTE) mbi.BaseAddress + mbi.RegionSize) < pbLast) {
            break;
        }

        // Skip uncommitted regions and guard pages.
        if ((mbi.State != MEM_COMMIT) ||
            ((mbi.Protect & 0xff) == PAGE_NOACCESS) ||
            (mbi.Protect & PAGE_GUARD)) {
            continue;
        }

        if (load_nt_header_from_process(
                hProcess, (HMODULE) pbLast, pNtHeader)) {
            return (HMODULE) pbLast;
        }
    }

    return NULL;
}
