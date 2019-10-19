#include "iidxhook-util/config-eamuse.h"

#include "test/check.h"
#include "test/test.h"

/* We don't care about cleaning up some of the memory to avoid cluttering
   the tests */

static void test_config_eamuse_defaults()
{
    struct cconfig *config;
    struct iidxhook_util_config_eamuse config_eamuse;

    config = cconfig_init();

    iidxhook_util_config_eamuse_init(config);
    iidxhook_util_config_eamuse_get(&config_eamuse, config);

    cconfig_finit(config);

    check_str_eq(config_eamuse.card_type, "C02");
    check_str_eq(net_addr_to_str(&config_eamuse.server), "localhost:80");
    check_str_eq(
        security_id_to_str(&config_eamuse.pcbid, false),
        "0101020304050607086F");
    check_str_eq(
        security_id_to_str(&config_eamuse.eamid, false),
        "0101020304050607086F");
}

static void test_config_eamuse()
{
    struct cconfig *config;
    struct iidxhook_util_config_eamuse config_eamuse;

    config = cconfig_init();

    iidxhook_util_config_eamuse_init(config);

    cconfig_set2(config, "eamuse.card_type", "D01");
    cconfig_set2(config, "eamuse.server", "http://myServer.com/legacy");
    cconfig_set2(config, "eamuse.pcbid", "0101020304050607086F");
    cconfig_set2(config, "eamuse.eamid", "0101020304050607086F");

    iidxhook_util_config_eamuse_get(&config_eamuse, config);

    cconfig_finit(config);

    check_str_eq(config_eamuse.card_type, "D01");
    check_str_eq(
        net_addr_to_str(&config_eamuse.server), "http://myServer.com/legacy");
    check_str_eq(
        security_id_to_str(&config_eamuse.pcbid, false),
        "0101020304050607086F");
    check_str_eq(
        security_id_to_str(&config_eamuse.eamid, false),
        "0101020304050607086F");
}

static void test_config_eamuse_invalid_values()
{
    struct cconfig *config;
    struct iidxhook_util_config_eamuse config_eamuse;

    config = cconfig_init();

    iidxhook_util_config_eamuse_init(config);

    cconfig_set2(config, "eamuse.card_type", "XXXX");
    cconfig_set2(config, "eamuse.server", "asdf");
    cconfig_set2(config, "eamuse.pcbid", "00000000000000000000");
    cconfig_set2(config, "eamuse.eamid", "00000000000000000000");

    iidxhook_util_config_eamuse_get(&config_eamuse, config);

    cconfig_finit(config);

    check_str_eq(config_eamuse.card_type, "C02");
    check_str_eq(net_addr_to_str(&config_eamuse.server), "asdf");
    check_str_eq(
        security_id_to_str(&config_eamuse.pcbid, false),
        "00000000000000000000");
    check_str_eq(
        security_id_to_str(&config_eamuse.eamid, false),
        "00000000000000000000");
}

TEST_MODULE_BEGIN("iidxhook-config-eamuse")
TEST_MODULE_TEST(test_config_eamuse_defaults)
TEST_MODULE_TEST(test_config_eamuse)
TEST_MODULE_TEST(test_config_eamuse_invalid_values)
TEST_MODULE_END()
