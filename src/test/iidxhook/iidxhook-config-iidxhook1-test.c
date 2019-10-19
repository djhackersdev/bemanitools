#include "iidxhook1/config-iidxhook1.h"

#include "test/check.h"
#include "test/test.h"

/* We don't care about cleaning up some of the memory to avoid cluttering the
 * tests */

static void test_config_iidxhook1_defaults()
{
    struct cconfig *config;
    struct iidxhook_config_iidxhook1 config_iidxhook1;

    config = cconfig_init();

    iidxhook_config_iidxhook1_init(config);
    iidxhook_config_iidxhook1_get(&config_iidxhook1, config);

    cconfig_finit(config);

    check_bool_false(config_iidxhook1.happy_sky_ms_bg_fix);
}

static void test_config_iidxhook1()
{
    struct cconfig *config;
    struct iidxhook_config_iidxhook1 config_iidxhook1;

    config = cconfig_init();

    iidxhook_config_iidxhook1_init(config);

    cconfig_set2(config, "misc.happy_sky_ms_bg_fix", "true");

    iidxhook_config_iidxhook1_get(&config_iidxhook1, config);

    cconfig_finit(config);

    check_bool_true(config_iidxhook1.happy_sky_ms_bg_fix);
}

static void test_config_iidxhook1_invalid_values()
{
    struct cconfig *config;
    struct iidxhook_config_iidxhook1 config_iidxhook1;

    config = cconfig_init();

    iidxhook_config_iidxhook1_init(config);

    cconfig_set2(config, "misc.happy_sky_ms_bg_fix", "123");

    iidxhook_config_iidxhook1_get(&config_iidxhook1, config);

    cconfig_finit(config);

    check_bool_false(config_iidxhook1.happy_sky_ms_bg_fix);
}

TEST_MODULE_BEGIN("iidxhook-config-iidxhook1")
TEST_MODULE_TEST(test_config_iidxhook1_defaults)
TEST_MODULE_TEST(test_config_iidxhook1)
TEST_MODULE_TEST(test_config_iidxhook1_invalid_values)
TEST_MODULE_END()
