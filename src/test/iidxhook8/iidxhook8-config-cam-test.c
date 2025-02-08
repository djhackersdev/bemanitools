#include "camhook/config-cam.h"

#include "test/check.h"
#include "test/test.h"

/* We don't care about cleaning up some of the memory to avoid cluttering the
 * tests */

static void test_config_cam_defaults()
{
    struct cconfig *config;
    struct camhook_config_cam config_cam;

    config = cconfig_init();

    camhook_config_cam_init(config, 2, false);
    camhook_config_cam_get(&config_cam, config, 2, false);

    cconfig_finit(config);

    check_bool_false(config_cam.disable_emu);
    check_str_eq(config_cam.device_id[0], "");
    check_str_eq(config_cam.device_id[1], "");
}

static void test_config_cam()
{
    struct cconfig *config;
    struct camhook_config_cam config_cam;

    config = cconfig_init();

    camhook_config_cam_init(config, 2, false);

    cconfig_set2(config, "cam.disable_emu", "true");
    cconfig_set2(config, "cam.device_id1", "asdjkasd");
    cconfig_set2(config, "cam.device_id2", "1234");

    camhook_config_cam_get(&config_cam, config, 2, false);

    cconfig_finit(config);

    check_bool_true(config_cam.disable_emu);
    check_str_eq(config_cam.device_id[0], "asdjkasd");
    check_str_eq(config_cam.device_id[1], "1234");
}

static void test_config_cam_invalid_values()
{
    struct cconfig *config;
    struct camhook_config_cam config_cam;

    config = cconfig_init();

    camhook_config_cam_init(config, 2, false);

    cconfig_set2(config, "cam.disable_emu", "123");
    cconfig_set2(config, "cam.device_id1", "asdjkasd");
    cconfig_set2(config, "cam.device_id2", "1234");

    camhook_config_cam_get(&config_cam, config, 2, false);

    cconfig_finit(config);

    check_bool_false(config_cam.disable_emu);
    check_str_eq(config_cam.device_id[0], "asdjkasd");
    check_str_eq(config_cam.device_id[1], "1234");
}

TEST_MODULE_BEGIN("camhook-config-cam")
TEST_MODULE_TEST(test_config_cam_defaults)
TEST_MODULE_TEST(test_config_cam)
TEST_MODULE_TEST(test_config_cam_invalid_values)
TEST_MODULE_END()
