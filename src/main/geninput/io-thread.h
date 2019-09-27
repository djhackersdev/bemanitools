#ifndef GENINPUT_IO_THREAD_H
#define GENINPUT_IO_THREAD_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

void io_thread_init(void);
void io_thread_add_device(const char *dev_node);
void io_thread_fini(void);

#endif
