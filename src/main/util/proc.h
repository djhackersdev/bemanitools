#pragma once

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define PROC_THREAD_CPU_AFFINITY_CORE(c) (1 << c)

struct proc_thread_info {
    int id;
    void* proc;
    // According to
    // https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreadpriority#return-value
    int priority;
    HMODULE origin_module;
};

void proc_terminate_current_process(uint32_t exit_code);

void* proc_thread_get_proc_address(int thread_id);

HMODULE proc_thread_proc_get_origin_module(void* proc_addr);

bool proc_thread_proc_get_origin_module_path(void* proc_addr, char* buffer, size_t len);

bool proc_thread_proc_get_origin_module_name(void* proc_addr, char* buffer, size_t len);

size_t proc_thread_scan_threads_current_process(struct proc_thread_info** info);

HANDLE proc_thread_get_handle(int thread_id);

bool proc_thread_set_priority(int thread_id, int priority);

// cpu starts with 0, specify multiple to set affinity to these cores
bool proc_thread_set_affinity(int thread_id, uint32_t cpu_mask);