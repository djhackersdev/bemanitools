#ifndef TEST_TEST_H
#define TEST_TEST_H

#include <stdio.h>

#include "util/log.h"

#define TEST_MODULE_BEGIN(name)                 \
    int main(int argc, char **argv)             \
    {                                           \
        log_to_writer(log_writer_stderr, NULL); \
        fprintf(stderr, "Executing test module '%s'...\n", #name);

#define TEST_MODULE_TEST(func)                              \
    {                                                       \
        fprintf(stderr, "\tRunning test '%s'...\n", #func); \
        func();                                             \
    }

#define TEST_MODULE_END()                                   \
    fprintf(stderr, "Finished execution of test module\n"); \
    return 0;                                               \
    }

#endif