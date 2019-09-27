#include <windows.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cconfig/cconfig-util.h"
#include "cconfig/cmd.h"

#include "inject/options.h"

#include "util/cmdline.h"
#include "util/mem.h"
#include "util/str.h"

static FILE* log_file = NULL;

static bool inject_dll(PROCESS_INFORMATION pi, const char* arg_dll);
static bool debug(HANDLE process, uint32_t pid);
static bool debug_wstr(HANDLE process, const OUTPUT_DEBUG_STRING_INFO *odsi);
static bool debug_str(HANDLE process, const OUTPUT_DEBUG_STRING_INFO *odsi);

int main(int argc, char **argv)
{
    struct options options;
    char *cmd_line;
    char dll_path[MAX_PATH];
    DWORD dll_path_length;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    BOOL ok;
    BOOL debug_ok;
    int hooks;
    int exec_arg_pos;

    options_init(&options);

    if (argc < 3 || !options_read_cmdline(&options, argc, argv)) {
        options_print_usage();
        goto usage_fail;
    }

    /* Open log file for logging if parameter specified */

    if (strlen(options.log_file) > 0) {
        log_file = fopen(options.log_file, "w+");

        if (!log_file) {
            fprintf(stderr, "Opening log file %s failed: %s\n", 
                options.log_file, strerror(errno));
            goto log_file_open_fail;
        }

        printf("Log file: %s\n", options.log_file);
    }

    /* Count hook dlls */
    hooks = 0;
    exec_arg_pos = 0;

    for (int i = 1; i < argc; i++) {
        if (str_ends_with(argv[i], "dll")) {
            hooks++;
        } else if (str_ends_with(argv[i], "exe")) {
            exec_arg_pos = i;
            break;
        }
    }

    if (!hooks) {
        fprintf(stderr, "No Hook DLL(s) specified before executable\n");

        goto hook_count_fail;
    }

    if (!exec_arg_pos) {
        fprintf(stderr, "No executable specified\n");

        goto find_exec_fail;
    }

    for (int i = 0; i < hooks; i++) {
        dll_path_length = SearchPath(NULL, argv[i + 1], NULL, MAX_PATH,
            dll_path, NULL);

        if (dll_path_length == 0) {
            fprintf(stderr, "Hook DLL not found: %08x\n",
                    (unsigned int) GetLastError());

            goto search_fail;
        }
    }


    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    cmd_line = args_join(argc - exec_arg_pos, argv + exec_arg_pos);

    ok = CreateProcess(argv[exec_arg_pos], cmd_line, NULL, NULL, FALSE,
        CREATE_SUSPENDED, NULL, NULL, &si, &pi);

    if (!ok) {
        fprintf(stderr, "Failed to launch hooked EXE: %08x\n",
                (unsigned int) GetLastError());

        goto start_fail;
    }

    free(cmd_line);
    cmd_line = NULL;

    for (int i = 0; i < hooks; i++) {
        if (!inject_dll(pi, argv[i + 1])) {
            goto inject_fail;
        }
    }

    debug_ok = false;

    if (options.debug && !options.remote_debugger) {
        debug_ok = DebugActiveProcess(pi.dwProcessId);

        if (!debug_ok) {
            fprintf(stderr, "DebugActiveProcess failed: %08x\n",
                    (unsigned int) GetLastError());
        } else {
            printf("Debug active process\n");
        }
    }

    if (options.remote_debugger) {
        printf("Waiting until debugger attaches to remote process...\n");

        while (true) {
            BOOL res = FALSE;

            if (!CheckRemoteDebuggerPresent(pi.hProcess, &res)) {
                fprintf(stderr, "CheckRemoteDebuggerPresent failed: %08x\n",
                    (unsigned int) GetLastError());
            }

            if (res) {
                printf("Debugger attached, resuming\n");
                break;
            }

            Sleep(1000);
        }
    }

    printf("Resuming remote process...\n");

    if (ResumeThread(pi.hThread) == -1) {
        fprintf(stderr, "Error restarting hooked process: %08x\n",
                (unsigned int) GetLastError());

        goto restart_fail;
    }

    CloseHandle(pi.hThread);

    if (options.debug) {
        if (!debug_ok || !debug(pi.hProcess, pi.dwProcessId)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
        }
    }

    CloseHandle(pi.hProcess);

    return EXIT_SUCCESS;

inject_fail:
restart_fail:
    TerminateProcess(pi.hProcess, EXIT_FAILURE);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

start_fail:
    if (cmd_line != NULL) {
        free(cmd_line);
    }

hook_count_fail:
find_exec_fail:
search_fail:
    if (log_file) {
        fclose(log_file);
    }

log_file_open_fail:
usage_fail:

    return EXIT_FAILURE;
}

static bool inject_dll(PROCESS_INFORMATION pi, const char* arg_dll)
{
    char dll_path[MAX_PATH];
    DWORD dll_path_length;
    void *remote_addr;
    BOOL ok;
    HANDLE remote_thread;

    printf("Injecting: %s\n", arg_dll);

    dll_path_length = SearchPath(NULL, arg_dll, NULL, MAX_PATH, dll_path,
        NULL);

    dll_path_length++;

    remote_addr = VirtualAllocEx(pi.hProcess, NULL, dll_path_length,
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (!remote_addr) {
        fprintf(stderr, "VirtualAllocEx failed: %08x\n",
                (unsigned int) GetLastError());

        goto alloc_fail;
    }

    ok = WriteProcessMemory(pi.hProcess, remote_addr, dll_path,
            dll_path_length, NULL);

    if (!ok) {
        fprintf(stderr, "WriteProcessMemory failed: %08x\n",
                (unsigned int) GetLastError());

        goto write_fail;
    }

    remote_thread = CreateRemoteThread(pi.hProcess, NULL, 0,
            (LPTHREAD_START_ROUTINE) LoadLibrary, remote_addr, 0, NULL);

    if (remote_thread == NULL) {
        fprintf(stderr, "CreateRemoteThread failed: %08x\n",
                (unsigned int) GetLastError());

        goto inject_fail;
    }

    WaitForSingleObject(remote_thread, INFINITE);
    CloseHandle(remote_thread);

    ok = VirtualFreeEx(pi.hProcess, remote_addr, 0, MEM_RELEASE);
    remote_addr = NULL;

    if (!ok) {
        fprintf(stderr, "VirtualFreeEx failed: %08x\n",
                (unsigned int) GetLastError());
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

static bool debug(HANDLE process, uint32_t pid)
{
    DEBUG_EVENT de;
    BOOL ok;

    for (;;) {
        ok = WaitForDebugEvent(&de, INFINITE);

        if (!ok) {
            fprintf(stderr, "WaitForDebugEvent failed: %08x\n",
                    (unsigned int) GetLastError());

            return false;
        }

        switch (de.dwDebugEventCode) {
            case CREATE_PROCESS_DEBUG_EVENT:
                CloseHandle(de.u.CreateProcessInfo.hFile);

                break;

            case EXIT_PROCESS_DEBUG_EVENT:
                if (de.dwProcessId == pid) {
                    return true;
                }

                break;

            case LOAD_DLL_DEBUG_EVENT:
                CloseHandle(de.u.LoadDll.hFile);

                break;

            case OUTPUT_DEBUG_STRING_EVENT:
                if (de.dwProcessId == pid) {
                    if (de.u.DebugString.fUnicode) {
                        if (!debug_wstr(process, &de.u.DebugString)) {
                            return false;
                        }
                    } else {
                        if (!debug_str(process, &de.u.DebugString)) {
                            return false;
                        }
                    }
                }

                break;
        }

        if (de.dwDebugEventCode == OUTPUT_DEBUG_STRING_EVENT) {
            ok = ContinueDebugEvent(de.dwProcessId, de.dwThreadId,
                    DBG_CONTINUE);
        } else {
            ok = ContinueDebugEvent(de.dwProcessId, de.dwThreadId,
                    DBG_EXCEPTION_NOT_HANDLED);
        }

        if (!ok) {
            fprintf(stderr, "ContinueDebugEvent failed: %08x\n",
                    (unsigned int) GetLastError());

            return false;
        }
    }
}

static char console_get_color(char* str)
{
    /* Add some color to make spotting warnings/errors easier.
        Based on debug output level identifier. */

    /* Avoids colored output on strings like "Windows" */
    if (str[1] != ':') {
        return 15;
    }

    switch (str[0]) {
        /* green */
        case 'M':
            return 10;
        /* blue */
        case 'I':
            return 9;
        /* yellow */
        case 'W':
            return 14;
        /* red */
        case 'F':
            return 12;
        /* default console color */
        default:
            return 15;
    }
}

static bool debug_wstr(HANDLE process, const OUTPUT_DEBUG_STRING_INFO *odsi)
{
    char *str;
    wchar_t *wstr;
    uint32_t nbytes;
    BOOL ok;

    nbytes = odsi->nDebugStringLength * sizeof(wchar_t);
    wstr = xmalloc(nbytes);

    ok = ReadProcessMemory(process, odsi->lpDebugStringData, wstr, nbytes,
            NULL);

    if (ok) {
        if (wstr_narrow(wstr, &str)) {
            str[odsi->nDebugStringLength - 1] = '\0';

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 
                console_get_color(str));
            printf("%s", str);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

            if (log_file) {
                fprintf(log_file, "%s", str);
            }

            free(str);
        } else {
            fprintf(stderr, "OutputDebugStringW: UTF-16 conversion failed\n");
        }
    } else {
        fprintf(stderr, "ReadProcessMemory failed: %08x\n",
                (unsigned int) GetLastError());

        return false;
    }

    free(wstr);

    return (bool) ok;
}

static bool debug_str(HANDLE process, const OUTPUT_DEBUG_STRING_INFO *odsi)
{
    char *str;
    BOOL ok;

    str = xmalloc(odsi->nDebugStringLength);

    ok = ReadProcessMemory(process, odsi->lpDebugStringData, str,
            odsi->nDebugStringLength, NULL);

    if (ok) {
        str[odsi->nDebugStringLength - 1] = '\0';

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 
            console_get_color(str));
        printf("%s", str);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

        if (log_file) {
            fprintf(log_file, "%s", str);
        }
    } else {
        fprintf(stderr, "ReadProcessMemory failed: %08x\n",
                (unsigned int) GetLastError());
    }

    free(str);

    return (bool) ok;
}

