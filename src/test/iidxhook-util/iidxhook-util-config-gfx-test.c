#include "iidxhook-util/config-gfx.h"

#include "test/check.h"
#include "test/test.h"

/* We don't care about cleaning up some of the memory to avoid cluttering 
   the tests */

static void test_config_gfx_defaults()
{
    struct cconfig* config;
    struct iidxhook_config_gfx config_gfx;

    config = cconfig_init();

    iidxhook_config_gfx_init(config);
    iidxhook_config_gfx_get(&config_gfx, config);

    cconfig_finit(config);

    check_bool_false(config_gfx.bgvideo_uv_fix);
    check_bool_false(config_gfx.framed);
    check_float_eq(config_gfx.frame_rate_limit, 0.0f, 0.1f);
    check_float_eq(config_gfx.monitor_check, -1.0f, 0.1f);
    check_int_eq(config_gfx.pci_id_vid, 0x1002);
    check_int_eq(config_gfx.pci_id_pid, 0x7146);
    check_bool_false(config_gfx.windowed);
    check_int_eq(config_gfx.scale_back_buffer_width, 0);
    check_int_eq(config_gfx.scale_back_buffer_height, 0);
    check_int_eq(config_gfx.scale_back_buffer_width, IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_NONE);
}

static void test_config_gfx()
{
    struct cconfig* config;
    struct iidxhook_config_gfx config_gfx;

    config = cconfig_init();

    iidxhook_config_gfx_init(config);

    cconfig_set2(config, "gfx.bgvideo_uv_fix", "true");
    cconfig_set2(config, "gfx.framed", "true");
    cconfig_set2(config, "gfx.frame_rate_limit", "60.04");
    cconfig_set2(config, "gfx.monitor_check", "59.999");
    cconfig_set2(config, "gfx.pci_id", "1111:1234");
    cconfig_set2(config, "gfx.windowed", "true");
    cconfig_set2(config, "gfx.scale_back_buffer_width", "1920");
    cconfig_set2(config, "gfx.scale_back_buffer_height", "1080");
    cconfig_set2(config, "gfx.scale_back_buffer_filter", "linear");

    iidxhook_config_gfx_get(&config_gfx, config);

    cconfig_finit(config);

    check_bool_true(config_gfx.bgvideo_uv_fix);
    check_bool_true(config_gfx.framed);
    check_float_eq(config_gfx.frame_rate_limit, 60.04f, 0.1f);
    check_float_eq(config_gfx.monitor_check, 59.999f, 0.001f);
    check_int_eq(config_gfx.pci_id_vid, 0x1111);
    check_int_eq(config_gfx.pci_id_pid, 0x1234);
    check_bool_true(config_gfx.windowed);
    check_int_eq(config_gfx.scale_back_buffer_width, 1920);
    check_int_eq(config_gfx.scale_back_buffer_height, 1080);
    check_int_eq(config_gfx.scale_back_buffer_filter, IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_LINEAR);
}

static void test_config_gfx_invalid_values()
{
    struct cconfig* config;
    struct iidxhook_config_gfx config_gfx;

    config = cconfig_init();

    iidxhook_config_gfx_init(config);

    cconfig_set2(config, "gfx.bgvideo_uv_fix", "123");
    cconfig_set2(config, "gfx.framed", "aaa");
    cconfig_set2(config, "gfx.frame_rate_limit", "-1.0f");
    cconfig_set2(config, "gfx.monitor_check", "asdf");
    cconfig_set2(config, "gfx.pci_id", "11111234");
    cconfig_set2(config, "gfx.windowed", "-5");
    cconfig_set2(config, "gfx.scale_back_buffer_width", "asdf");
    cconfig_set2(config, "gfx.scale_back_buffer_height", "");
    cconfig_set2(config, "gfx.scale_back_buffer_filter", "ffff");

    iidxhook_config_gfx_get(&config_gfx, config);

    cconfig_finit(config);

    check_bool_false(config_gfx.bgvideo_uv_fix);
    check_bool_false(config_gfx.framed);
    check_float_eq(config_gfx.frame_rate_limit, 0.0f, 0.1f);
    check_float_eq(config_gfx.monitor_check, -1.0f, 0.1f);
    check_int_eq(config_gfx.pci_id_vid, 0x1002);
    check_int_eq(config_gfx.pci_id_pid, 0x7146);
    check_bool_false(config_gfx.windowed);
    check_int_eq(config_gfx.scale_back_buffer_width, 0);
    check_int_eq(config_gfx.scale_back_buffer_height, 0);
    check_int_eq(config_gfx.scale_back_buffer_width, IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_NONE);
}

TEST_MODULE_BEGIN("iidxhook-config-gfx")
TEST_MODULE_TEST(test_config_gfx_defaults)
TEST_MODULE_TEST(test_config_gfx)
TEST_MODULE_TEST(test_config_gfx_invalid_values)
TEST_MODULE_END()
