#include <windows.h>

#include <stdint.h>

void proc_terminate_current_process(uint32_t exit_code)
{
    HANDLE hnd;

    hnd = OpenProcess(
        SYNCHRONIZE | PROCESS_TERMINATE,
        TRUE,
        GetCurrentProcessId());
        
    TerminateProcess(hnd, 0);
}