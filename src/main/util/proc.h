#pragma once

#include <stdbool.h>
#include <stdint.h>

bool proc_is_running_as_admin_user();

void proc_terminate_current_process(uint32_t exit_code);

bool proc_is_module_loaded(const char *name);