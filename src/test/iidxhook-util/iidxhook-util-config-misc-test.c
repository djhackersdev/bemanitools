#include "iidxhook-util/config-misc.h"

#include "test/check.h"
#include "test/test.h"

/* We don't care about cleaning up some of the memory to avoid cluttering the
 * tests */

static void test_config_misc_defaults()
{
    struct cconfig *config;
    struct iidxhook_config_misc config_misc;

    config = cconfig_init();

    iidxhook_config_misc_init(config);
    iidxhook_config_misc_get(&config_misc, config);

    cconfig_finit(config);

    check_bool_false(config_misc.disable_clock_set);
    check_bool_false(config_misc.rteffect_stub);
}

static void test_config_misc()
{
    struct cconfig *config;
    struct iidxhook_config_misc config_misc;

    config = cconfig_init();

    iidxhook_config_misc_init(config);

    cconfig_set2(config, "misc.disable_clock_set", "true");
    cconfig_set2(config, "misc.rteffect_stub", "true");

    iidxhook_config_misc_get(&config_misc, config);

    cconfig_finit(config);

    check_bool_true(config_misc.disable_clock_set);
    check_bool_true(config_misc.rteffect_stub);
}

static void test_config_misc_invalid_values()
{
    struct cconfig *config;
    struct iidxhook_config_misc config_misc;

    config = cconfig_init();

    iidxhook_config_misc_init(config);

    cconfig_set2(config, "misc.disable_clock_set", "asdf");
    cconfig_set2(config, "misc.rteffect_stub", "222");

    iidxhook_config_misc_get(&config_misc, config);

    cconfig_finit(config);

    check_bool_false(config_misc.disable_clock_set);
    check_bool_false(config_misc.rteffect_stub);
}

TEST_MODULE_BEGIN("iidxhook-config-misc")
TEST_MODULE_TEST(test_config_misc_defaults)
TEST_MODULE_TEST(test_config_misc)
TEST_MODULE_TEST(test_config_misc_invalid_values)
TEST_MODULE_END()
