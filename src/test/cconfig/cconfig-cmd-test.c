#include "cconfig/cconfig-util.h"
#include "cconfig/cmd.h"

#include "test/check.h"
#include "test/test.h"

static void test_cmd()
{
    struct cconfig *config;
    int argc = 6;
    char *argv[] = {"-P", "test=aaa", "-C", "asdf", "-P", "test2=123"};
    char str[32];
    int32_t val;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    check_bool_true(cconfig_cmd_parse(config, "-P", argc, argv, true));

    check_int_eq(config->nentries, 2);

    check_bool_true(
        cconfig_util_get_str(config, "test", str, sizeof(str), "1"));
    check_str_eq(str, "aaa");

    check_bool_true(cconfig_util_get_int(config, "test2", &val, 0));
    check_int_eq(val, 123);

    cconfig_finit(config);
}

static void test_cmd_absent_entries()
{
    struct cconfig *config;
    int argc = 6;
    char *argv[] = {"-P", "test=aaa", "-C", "asdf", "-P", "test2=123"};
    char str[32];
    int32_t val;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    cconfig_set2(config, "test", "aaa");

    check_bool_true(cconfig_cmd_parse(config, "-P", argc, argv, false));

    check_int_eq(config->nentries, 1);

    check_bool_true(
        cconfig_util_get_str(config, "test", str, sizeof(str), "1"));
    check_str_eq(str, "aaa");

    check_bool_false(cconfig_util_get_int(config, "test2", &val, 0));
    check_int_eq(val, 0);

    cconfig_finit(config);
}

TEST_MODULE_BEGIN("cconfig-cmd")
TEST_MODULE_TEST(test_cmd)
TEST_MODULE_TEST(test_cmd_absent_entries)
TEST_MODULE_END()
