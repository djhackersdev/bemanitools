#ifndef LAUNCHER_AVS_H
#define LAUNCHER_AVS_H

#include <stdint.h>

#include "core/property-node.h"

void avs_fs_assert_root_device_exists(const core_property_node_t *node);
void avs_fs_mountpoints_fs_dirs_create(const core_property_node_t *node);
void avs_init(
    const core_property_node_t *node,
    uint32_t avs_heap_size,
    uint32_t std_heap_size);
void avs_fs_file_copy(const char *src, const char *dst);
void avs_fs_dir_log(const char *path);
void avs_fini(void);

#endif
