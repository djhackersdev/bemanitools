#include "iidxhook2/config-iidxhook2.h"

#include "test/check.h"
#include "test/test.h"

/* We don't care about cleaning up some of the memory to avoid cluttering the tests */

static void test_config_iidxhook2_defaults()
{
    struct cconfig* config;
    struct iidxhook_config_iidxhook2 config_iidxhook2;

    config = cconfig_init();

    iidxhook_config_iidxhook2_init(config);
    iidxhook_config_iidxhook2_get(&config_iidxhook2, config);

    cconfig_finit(config);

    check_bool_false(config_iidxhook2.distorted_ms_bg_fix);
}

static void test_config_iidxhook2()
{
    struct cconfig* config;
    struct iidxhook_config_iidxhook2 config_iidxhook2;

    config = cconfig_init();

    iidxhook_config_iidxhook2_init(config);

    cconfig_set2(config, "misc.distorted_ms_bg_fix", "true");

    iidxhook_config_iidxhook2_get(&config_iidxhook2, config);

    cconfig_finit(config);

    check_bool_true(config_iidxhook2.distorted_ms_bg_fix);
}

static void test_config_iidxhook2_invalid_values()
{
    struct cconfig* config;
    struct iidxhook_config_iidxhook2 config_iidxhook2;

    config = cconfig_init();

    iidxhook_config_iidxhook2_init(config);

    cconfig_set2(config, "misc.distorted_ms_bg_fix", "123");

    iidxhook_config_iidxhook2_get(&config_iidxhook2, config);

    cconfig_finit(config);

    check_bool_false(config_iidxhook2.distorted_ms_bg_fix);
}

TEST_MODULE_BEGIN("iidxhook-config-iidxhook2")
TEST_MODULE_TEST(test_config_iidxhook2_defaults)
TEST_MODULE_TEST(test_config_iidxhook2)
TEST_MODULE_TEST(test_config_iidxhook2_invalid_values)
TEST_MODULE_END()
