#ifndef CONFIG_SNAP_H
#define CONFIG_SNAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "geninput/mapper.h"

struct snap_dev {
    struct hid_stub *hid;
    int32_t *states;
    struct hid_control *controls;
    uint32_t ncontrols;
};

struct snap {
    struct snap_dev *devs;
    uint32_t ndevs;
};

void snap_init(struct snap *snap);
bool snap_find_edge(
    const struct snap *snap,
    const struct snap *other_snap,
    struct mapped_action *ma);
void snap_fini(struct snap *snap);

#endif
