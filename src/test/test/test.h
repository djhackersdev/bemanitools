#ifndef TEST_TEST_H
#define TEST_TEST_H

#include <stdio.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"

#include "iface-core/log.h"

#define TEST_MODULE_BEGIN(name)                            \
    int main(int argc, char **argv)                        \
    {                                                      \
        core_log_bt_core_api_set();                        \
                                                           \
        core_log_bt_ext_init_with_stderr();                \
        core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC); \
        fprintf(stderr, "Executing test module '%s'...\n", #name);

#define TEST_MODULE_TEST(func)                              \
    {                                                       \
        fprintf(stderr, "\tRunning test '%s'...\n", #func); \
        func();                                             \
    }

#define TEST_MODULE_END()                                   \
    fprintf(stderr, "Finished execution of test module\n"); \
    core_log_bt_fini();                                     \
                                                            \
    return 0;                                               \
    }

#endif