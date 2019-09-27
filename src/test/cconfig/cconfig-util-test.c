#include "cconfig/cconfig-util.h"
#include "cconfig/cconfig.h"

#include "test/check.h"
#include "test/test.h"

static void test_set_get_int()
{
    struct cconfig* config;
    int32_t value;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    cconfig_util_set_int(config, "test", 12345, "desc");

    check_int_eq(config->nentries, 1);
    check_str_eq(config->entries[0].key, "test");
    check_str_eq(config->entries[0].value, "12345");
    check_str_eq(config->entries[0].desc, "desc");

    check_bool_true(cconfig_util_get_int(config, "test", &value, 0));
    check_int_eq(value, 12345);

    cconfig_finit(config);
}

static void test_get_int_na()
{
    struct cconfig* config;
    int32_t value;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    check_bool_false(cconfig_util_get_int(config, "test", &value, 123));
    check_int_eq(value, 123);

    cconfig_finit(config);
}

static void test_set_get_float()
{
    struct cconfig* config;
    float value;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    cconfig_util_set_float(config, "test", 100.5f, "desc");

    check_int_eq(config->nentries, 1);
    check_str_eq(config->entries[0].key, "test");
    check_str_eq(config->entries[0].desc, "desc");

    check_bool_true(cconfig_util_get_float(config, "test", &value, 0));
    check_float_eq(value, 100.5f, 0.1f);

    cconfig_finit(config);
}

static void test_get_float_na()
{
    struct cconfig* config;
    float value;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    check_bool_false(cconfig_util_get_float(config, "test", &value, 115.9f));
    check_float_eq(value, 115.9f, 0.1f);

    cconfig_finit(config);
}

static void test_set_get_bool()
{
    struct cconfig* config;
    bool value;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    cconfig_util_set_bool(config, "test", true, "desc");

    check_int_eq(config->nentries, 1);
    check_str_eq(config->entries[0].key, "test");
    check_str_eq(config->entries[0].desc, "desc");

    check_bool_true(cconfig_util_get_bool(config, "test", &value, 0));
    check_bool_true(value);

    cconfig_finit(config);
}

static void test_get_bool_na()
{
    struct cconfig* config;
    bool value;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    check_bool_false(cconfig_util_get_bool(config, "test", &value, true));
    check_bool_true(value);

    cconfig_finit(config);
}

static void test_set_get_str()
{
    struct cconfig* config;
    char value[1024];

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    cconfig_util_set_str(config, "test", "hello world", "desc");

    check_int_eq(config->nentries, 1);
    check_str_eq(config->entries[0].key, "test");
    check_str_eq(config->entries[0].desc, "desc");

    check_bool_true(cconfig_util_get_str(config, "test", value, sizeof(value),
        "world hello"));
    check_str_eq(value, "hello world");

    cconfig_finit(config);
}

static void test_get_str_na()
{
    struct cconfig* config;
    char value[1024];

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    check_bool_false(cconfig_util_get_str(config, "test", value, sizeof(value), 
        "world hello"));
    check_str_eq(value, "world hello");

    cconfig_finit(config);
}

static void test_set_get_data()
{
    struct cconfig* config;
    uint8_t value[1024];
    uint8_t value2[1024];
    uint8_t value_default[] = {0x00};

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    for (int i = 0; i < sizeof(value); i++) {
        value[i] = (uint8_t) i;
    }

    cconfig_util_set_data(config, "test", value, sizeof(value), "desc");

    check_int_eq(config->nentries, 1);
    check_str_eq(config->entries[0].key, "test");
    check_str_eq(config->entries[0].desc, "desc");

    check_bool_true(cconfig_util_get_data(config, "test", value2, sizeof(value2),
        value_default));
    check_data_eq(value2, sizeof(value2), value, sizeof(value));

    cconfig_finit(config);
}

static void test_get_data_na()
{
    struct cconfig* config;
    uint8_t value[1024];
    uint8_t value_default[] = {0x01};

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    check_bool_false(cconfig_util_get_data(config, "test", value, sizeof(value), 
        value_default));
    check_data_eq(value, 1, value_default, sizeof(value_default));

    cconfig_finit(config);
}

TEST_MODULE_BEGIN("cconfig-util")
TEST_MODULE_TEST(test_set_get_int)
TEST_MODULE_TEST(test_get_int_na)
TEST_MODULE_TEST(test_set_get_float)
TEST_MODULE_TEST(test_get_float_na)
TEST_MODULE_TEST(test_set_get_bool)
TEST_MODULE_TEST(test_get_bool_na)
TEST_MODULE_TEST(test_set_get_str)
TEST_MODULE_TEST(test_get_str_na)
TEST_MODULE_TEST(test_set_get_data)
TEST_MODULE_TEST(test_get_data_na)
TEST_MODULE_END()
