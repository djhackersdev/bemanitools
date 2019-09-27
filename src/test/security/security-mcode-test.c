#include "security/mcode.h"

#include "test/check.h"
#include "test/test.h"

static void test_parse_valid()
{
    struct security_mcode mcode;

    check_bool_true(security_mcode_parse("GQD01JAA", &mcode));
    check_char_eq(mcode.header, 'G');
    check_char_eq(mcode.unkn, 'Q');
    check_data_eq(mcode.game, sizeof(mcode.game), "D01", 3);
    check_char_eq(mcode.region, 'J');
    check_char_eq(mcode.cabinet, 'A');
    check_char_eq(mcode.revision, 'A');
}

static void test_parse_valid_2()
{
    struct security_mcode mcode;

    check_bool_true(security_mcode_parse("GQD01", &mcode));
    check_char_eq(mcode.header, 'G');
    check_char_eq(mcode.unkn, 'Q');
    check_data_eq(mcode.game, sizeof(mcode.game), "D01", 3);
    check_char_eq(mcode.region, ' ');
    check_char_eq(mcode.cabinet, ' ');
    check_char_eq(mcode.revision, ' ');
}

static void test_parse_invalid()
{
    struct security_mcode mcode;

    check_bool_false(security_mcode_parse("GQD01JAAA", &mcode));
    check_char_eq(mcode.header, ' ');
    check_char_eq(mcode.unkn, ' ');
    check_data_eq(mcode.game, sizeof(mcode.game), "   ", 3);
    check_char_eq(mcode.region, ' ');
    check_char_eq(mcode.cabinet, ' ');
    check_char_eq(mcode.revision, ' ');
}

static void test_to_str()
{
    struct security_mcode mcode;
    mcode.header = 'G';
    mcode.unkn = 'Q';
    memcpy(mcode.game, "D01", 3);
    mcode.region = 'J';
    mcode.cabinet = 'A';
    mcode.revision = 'A';

    check_str_eq(security_mcode_to_str(&mcode), "GQD01JAA");
}

TEST_MODULE_BEGIN("security-mcode")
TEST_MODULE_TEST(test_parse_valid)
TEST_MODULE_TEST(test_parse_valid_2)
TEST_MODULE_TEST(test_parse_invalid)
TEST_MODULE_TEST(test_to_str)
TEST_MODULE_END()
