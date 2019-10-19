#include "iidxhook-util/config-sec.h"

#include "test/check.h"
#include "test/test.h"

/* We don't care about cleaning up some of the memory to avoid cluttering the
 * tests */

static void test_config_sec_defaults()
{
    struct cconfig *config;
    struct iidxhook_config_sec config_sec;
    const uint32_t boot_seeds[3] = {0, 0, 0};

    config = cconfig_init();

    iidxhook_config_sec_init(config);
    iidxhook_config_sec_get(&config_sec, config);

    cconfig_finit(config);

    check_str_eq(security_mcode_to_str(&config_sec.boot_version), "GEC02   ");
    check_data_eq(
        config_sec.boot_seeds,
        sizeof(config_sec.boot_seeds),
        (void *) boot_seeds,
        sizeof(boot_seeds));
    check_str_eq(
        security_mcode_to_str(&config_sec.black_plug_mcode), "GQC02JAA");
}

static void test_config_sec()
{
    struct cconfig *config;
    struct iidxhook_config_sec config_sec;
    const uint32_t boot_seeds[3] = {1, 1, 1};

    config = cconfig_init();

    iidxhook_config_sec_init(config);

    cconfig_set2(config, "sec.boot_version", "ASDFG   ");
    cconfig_set2(config, "sec.boot_seeds", "1:1:1");
    cconfig_set2(config, "sec.black_plug_mcode", "GQD01JAB");

    iidxhook_config_sec_get(&config_sec, config);

    cconfig_finit(config);

    check_str_eq(security_mcode_to_str(&config_sec.boot_version), "ASDFG   ");
    check_data_eq(
        config_sec.boot_seeds,
        sizeof(config_sec.boot_seeds),
        (void *) boot_seeds,
        sizeof(boot_seeds));
    check_str_eq(
        security_mcode_to_str(&config_sec.black_plug_mcode), "GQD01JAB");
}

static void test_config_sec_invalid_values()
{
    struct cconfig *config;
    struct iidxhook_config_sec config_sec;
    const uint32_t boot_seeds[3] = {0, 0, 0};

    config = cconfig_init();

    iidxhook_config_sec_init(config);

    cconfig_set2(config, "sec.boot_version", "ASDFG1234");
    cconfig_set2(config, "sec.boot_seeds", "1:11");
    cconfig_set2(config, "sec.black_plug_mcode", "GQD01JABA");

    iidxhook_config_sec_get(&config_sec, config);

    cconfig_finit(config);

    check_str_eq(security_mcode_to_str(&config_sec.boot_version), "GEC02   ");
    check_data_eq(
        config_sec.boot_seeds,
        sizeof(config_sec.boot_seeds),
        (void *) boot_seeds,
        sizeof(boot_seeds));
    check_str_eq(
        security_mcode_to_str(&config_sec.black_plug_mcode), "GQC02JAA");
}

TEST_MODULE_BEGIN("iidxhook-config-sec")
TEST_MODULE_TEST(test_config_sec_defaults)
TEST_MODULE_TEST(test_config_sec)
TEST_MODULE_TEST(test_config_sec_invalid_values)
TEST_MODULE_END()
