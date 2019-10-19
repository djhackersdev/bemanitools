#ifndef JBHOOK_OPTIONS_H
#define JBHOOK_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

struct options {
    bool windowed;
    bool window_framed;
    bool disable_p4ioemu;
    bool disable_cardemu;
};

void options_init_from_cmdline(struct options *options);

void options_init(struct options *options);
bool options_read_cmdline(struct options *options, int argc, const char **argv);
void options_print_usage(void);
void options_fini(struct options *options);

#endif
