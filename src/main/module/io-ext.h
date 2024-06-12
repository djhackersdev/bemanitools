#ifndef MODULE_IO_EXT_H
#define MODULE_IO_EXT_H

#include "module/io.h"

void module_io_ext_load_and_init(
    const char *path, const char *api_get_func_name, module_io_t **module);

#endif