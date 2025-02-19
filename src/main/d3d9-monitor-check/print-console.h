#ifndef PRINT_CONSOLE_H
#define PRINT_CONSOLE_H

#include <stdio.h>

#include "util/winerr.h"

#define printf_out(fmt, ...) \
    fprintf(stdout, fmt, ##__VA_ARGS__)
#define printf_err(fmt, ...) \
    fprintf(stderr, fmt, ##__VA_ARGS__)
#define printfln_out(fmt, ...) \
    fprintf(stdout, fmt "\n", ##__VA_ARGS__)
#define printfln_err(fmt, ...) \
    fprintf(stderr, fmt "\n", ##__VA_ARGS__)

#define printfln_winerr(fmt, ...) \
    char *winerr = util_winerr_format_last_error_code(); \
    fprintf(stderr, fmt ": %s\n", ##__VA_ARGS__, winerr); \
    free(winerr);

#endif
