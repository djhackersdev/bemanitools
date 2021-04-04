#include <stdbool.h>

#include "acio/h44b.h"

bool jb_io_h44b_init(const char *port, int32_t baud);
bool jb_io_h44b_fini(void);
bool jb_io_h44b_write_lights(struct ac_io_h44b_output *lights);
