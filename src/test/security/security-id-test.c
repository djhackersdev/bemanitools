#include "security/id.h"

#include "test/check.h"
#include "test/test.h"

static void test_to_str()
{
    check_str_eq(security_id_to_str(&security_id_default, false), 
        "0101020304050607086F");
    check_str_eq(security_id_to_str(&security_id_default, true), 
        "0102030405060708");
}

static void test_parse_valid()
{
    struct security_id id;

    check_bool_true(security_id_parse("0101020304050607086F", &id));
    check_bool_true(security_id_verify(&id));
}

static void test_parse_invalid()
{
    struct security_id id;

    check_bool_true(security_id_parse("0102030405060708", &id));
    check_bool_false(security_id_verify(&id));

    check_bool_true(security_id_parse("010102030405060708FF", &id));
    check_bool_false(security_id_verify(&id));

    check_bool_false(security_id_parse("asdf", &id));
}

static void test_prepare_and_verify()
{
    struct security_id id;

    check_bool_true(security_id_parse("01AB50E2DC4F832A9A78", &id));
    check_bool_true(security_id_verify(&id));
}

static void test_prepare_and_verify_invalid()
{
    struct security_id id;

    check_bool_true(security_id_parse("01AB50E2DC4F832A9A79", &id));
    check_bool_false(security_id_verify(&id));
}

TEST_MODULE_BEGIN("security-id")
TEST_MODULE_TEST(test_to_str)
TEST_MODULE_TEST(test_parse_valid)
TEST_MODULE_TEST(test_parse_invalid)
TEST_MODULE_TEST(test_prepare_and_verify)
TEST_MODULE_TEST(test_prepare_and_verify_invalid)
TEST_MODULE_END()
