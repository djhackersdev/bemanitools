#ifndef BIO2_BIO2
#define BIO2_BIO2

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum bio2_bi2a_cmd {
    // Same as AC_IO_CMD_CLEAR but called differently
    // with a parameter required to initialize the IO correctly
    BIO2_BI2A_CMD_INIT = 0x0100,
    BIO2_BI2A_CMD_WATCHDOG = 0x0120,
    BIO2_BI2A_CMD_POLL = 0x0152, // For IIDX
};

#endif