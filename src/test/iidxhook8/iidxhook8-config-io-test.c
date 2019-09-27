#include "iidxhook8/config-io.h"

#include "test/check.h"
#include "test/test.h"

/* We don't care about cleaning up some of the memory to avoid cluttering the tests */

static void test_config_io_defaults()
{
    struct cconfig* config;
    struct iidxhook8_config_io config_io;

    config = cconfig_init();

    iidxhook8_config_io_init(config);
    iidxhook8_config_io_get(&config_io, config);

    cconfig_finit(config);

    check_bool_false(config_io.disable_card_reader_emu);
    check_bool_false(config_io.disable_bio2_emu);
    check_bool_false(config_io.disable_poll_limiter);
}

static void test_config_io()
{
    struct cconfig* config;
    struct iidxhook8_config_io config_io;

    config = cconfig_init();

    iidxhook8_config_io_init(config);

    cconfig_set2(config, "io.disable_card_reader_emu", "true");
    cconfig_set2(config, "io.disable_bio2_emu", "true");
    cconfig_set2(config, "io.disable_poll_limiter", "true");

    iidxhook8_config_io_get(&config_io, config);

    cconfig_finit(config);

    check_bool_true(config_io.disable_card_reader_emu);
    check_bool_true(config_io.disable_bio2_emu);
    check_bool_true(config_io.disable_poll_limiter);
}

static void test_config_invalid_values()
{
   struct cconfig* config;
    struct iidxhook8_config_io config_io;

    config = cconfig_init();

    iidxhook8_config_io_init(config);

    cconfig_set2(config, "io.disable_card_reader_emu", "asdf");
    cconfig_set2(config, "io.disable_bio2_emu", "123");
    cconfig_set2(config, "io.disable_poll_limiter", "");

    iidxhook8_config_io_get(&config_io, config);

    cconfig_finit(config);

    check_bool_false(config_io.disable_card_reader_emu);
    check_bool_false(config_io.disable_bio2_emu);
    check_bool_false(config_io.disable_poll_limiter);
}

TEST_MODULE_BEGIN("iidxhook8-config-io")
TEST_MODULE_TEST(test_config_io_defaults)
TEST_MODULE_TEST(test_config_io)
TEST_MODULE_TEST(test_config_io_invalid_values)
TEST_MODULE_END()
