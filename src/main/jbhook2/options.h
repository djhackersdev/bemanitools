#ifndef JBHOOK_OPTIONS_H
#define JBHOOK_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

struct options {
    bool windowed;
    bool vertical;
    bool disable_p3ioemu;
    bool disable_cardemu;
    bool disable_adapteremu;
    bool show_cursor;
};

void options_init_from_cmdline(struct options *options);

void options_init(struct options *options);
bool options_read_cmdline(struct options *options, int argc, const char **argv);
void options_print_usage(void);
void options_fini(struct options *options);

#endif
