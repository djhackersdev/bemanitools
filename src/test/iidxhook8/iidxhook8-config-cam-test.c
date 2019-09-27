#include "iidxhook8/config-cam.h"

#include "test/check.h"
#include "test/test.h"

/* We don't care about cleaning up some of the memory to avoid cluttering the tests */

static void test_config_cam_defaults()
{
    struct cconfig* config;
    struct iidxhook8_config_cam config_cam;

    config = cconfig_init();

    iidxhook8_config_cam_init(config);
    iidxhook8_config_cam_get(&config_cam, config);

    cconfig_finit(config);

    check_bool_false(config_cam.disable_emu);
    check_str_eq(config_cam.device_id1, "");
    check_str_eq(config_cam.device_id2, "");
}

static void test_config_cam()
{
    struct cconfig* config;
    struct iidxhook8_config_cam config_cam;

    config = cconfig_init();

    iidxhook8_config_cam_init(config);

    cconfig_set2(config, "cam.disable_emu", "true");
    cconfig_set2(config, "cam.device_id1", "asdjkasd");
    cconfig_set2(config, "cam.device_id2", "1234");

    iidxhook8_config_cam_get(&config_cam, config);

    cconfig_finit(config);

    check_bool_true(config_cam.disable_emu);
    check_str_eq(config_cam.device_id1, "asdjkasd");
    check_str_eq(config_cam.device_id2, "1234");
}

static void test_config_invalid_values()
{
    struct cconfig* config;
    struct iidxhook8_config_cam config_cam;

    config = cconfig_init();

    iidxhook8_config_cam_init(config);

    cconfig_set2(config, "cam.disable_emu", "123");
    cconfig_set2(config, "cam.device_id1", "asdjkasd");
    cconfig_set2(config, "cam.device_id2", "1234");

    iidxhook8_config_cam_get(&config_cam, config);

    cconfig_finit(config);

    check_bool_fakse(config_cam.disable_emu);
    check_str_eq(config_cam.device_id1, "asdjkasd");
    check_str_eq(config_cam.device_id2, "1234");
}

TEST_MODULE_BEGIN("iidxhook8-config-cam")
TEST_MODULE_TEST(test_config_cam_defaults)
TEST_MODULE_TEST(test_config_cam)
TEST_MODULE_TEST(test_config_cam_invalid_values)
TEST_MODULE_END()
