#include "cconfig/cconfig.h"

#include "test/check.h"
#include "test/test.h"

static void test_init()
{
    struct cconfig *config;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    cconfig_finit(config);
}

static void test_get_empty()
{
    struct cconfig *config;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    check_null(cconfig_get(config, "test"));

    cconfig_finit(config);
}

static void test_set_get_one_elem()
{
    struct cconfig *config;
    struct cconfig_entry *entry;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    cconfig_set(config, "test", "123", "desc");

    check_int_eq(config->nentries, 1);
    check_str_eq(config->entries[0].key, "test");
    check_str_eq(config->entries[0].value, "123");
    check_str_eq(config->entries[0].desc, "desc");

    entry = cconfig_get(config, "test");

    check_str_eq(entry->key, "test");
    check_str_eq(entry->value, "123");
    check_str_eq(entry->desc, "desc");

    cconfig_finit(config);
}

static void test_set_get_one_elem2()
{
    struct cconfig *config;
    struct cconfig_entry *entry;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    cconfig_set2(config, "test", "123");

    check_int_eq(config->nentries, 1);
    check_str_eq(config->entries[0].key, "test");
    check_str_eq(config->entries[0].value, "123");
    check_str_eq(config->entries[0].desc, "");

    entry = cconfig_get(config, "test");

    check_str_eq(entry->key, "test");
    check_str_eq(entry->value, "123");
    check_str_eq(entry->desc, "");

    cconfig_finit(config);
}

static void test_set_get_same_elem()
{
    struct cconfig *config;
    struct cconfig_entry *entry;

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    cconfig_set(config, "test", "123", "desc");
    cconfig_set(config, "test", "12345", "aaa");

    check_int_eq(config->nentries, 1);

    check_str_eq(config->entries[0].key, "test");
    check_str_eq(config->entries[0].value, "12345");
    check_str_eq(config->entries[0].desc, "aaa");

    entry = cconfig_get(config, "test");

    check_str_eq(entry->key, "test");
    check_str_eq(entry->value, "12345");
    check_str_eq(entry->desc, "aaa");

    cconfig_finit(config);
}

static void test_set_get_many_elem()
{
    struct cconfig *config;
    struct cconfig_entry *entry;
    char key[16];

    config = cconfig_init();

    check_int_eq(config->nentries, 0);
    check_null(config->entries);

    for (int i = 0; i < 1000; i++) {
        sprintf(key, "%d", i);
        cconfig_set(config, key, "123", "desc");
    }

    check_int_eq(config->nentries, 1000);

    for (int i = 0; i < 1000; i++) {
        sprintf(key, "%d", i);
        check_str_eq(config->entries[i].key, key);
        check_str_eq(config->entries[i].value, "123");
        check_str_eq(config->entries[i].desc, "desc");
    }

    for (int i = 0; i < 1000; i++) {
        sprintf(key, "%d", i);
        entry = cconfig_get(config, key);

        check_str_eq(entry->key, key);
        check_str_eq(entry->value, "123");
        check_str_eq(entry->desc, "desc");
    }

    cconfig_finit(config);
}

TEST_MODULE_BEGIN("cconfig")
TEST_MODULE_TEST(test_init)
TEST_MODULE_TEST(test_get_empty)
TEST_MODULE_TEST(test_set_get_one_elem)
TEST_MODULE_TEST(test_set_get_one_elem2)
TEST_MODULE_TEST(test_set_get_same_elem)
TEST_MODULE_TEST(test_set_get_many_elem)
TEST_MODULE_END()
