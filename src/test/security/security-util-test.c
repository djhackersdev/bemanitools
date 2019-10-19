#include "security/mcode.h"
#include "security/util.h"

#include "test/check.h"
#include "test/test.h"

static const struct security_mcode black_plug_mcode_jubeat = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_C,
    .game = SECURITY_MCODE_GAME_JB_1,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_B,
};

static void test_mcode_encode_decode_eamuse()
{
    uint8_t buffer[8];
    uint8_t result[8];

    security_util_8_to_6_encode(
        (const uint8_t *) &security_mcode_eamuse, buffer);
    security_util_6_to_8_decode(buffer, result);

    check_data_eq(
        result,
        sizeof(result),
        &security_mcode_eamuse,
        sizeof(security_mcode_eamuse));
}

static void test_mcode_encode_decode_jubeat()
{
    uint8_t buffer[8];
    uint8_t result[8];

    security_util_8_to_6_encode(
        (const uint8_t *) &black_plug_mcode_jubeat, buffer);
    security_util_6_to_8_decode(buffer, result);

    check_data_eq(
        result,
        sizeof(result),
        &black_plug_mcode_jubeat,
        sizeof(black_plug_mcode_jubeat));
}

static void test_mcode_encode_decode_reverse_eamuse()
{
    uint8_t buffer[8];
    uint8_t result[8];

    security_util_8_to_6_encode_reverse(
        (const uint8_t *) &security_mcode_eamuse, buffer);
    security_util_6_to_8_decode_reverse(buffer, result);

    check_data_eq(
        result,
        sizeof(result),
        &security_mcode_eamuse,
        sizeof(security_mcode_eamuse));
}

static void test_mcode_encode_decode_reverse_jubeat()
{
    uint8_t buffer[8];
    uint8_t result[8];

    security_util_8_to_6_encode_reverse(
        (const uint8_t *) &black_plug_mcode_jubeat, buffer);
    security_util_6_to_8_decode_reverse(buffer, result);

    check_data_eq(
        result,
        sizeof(result),
        &black_plug_mcode_jubeat,
        sizeof(black_plug_mcode_jubeat));
}

TEST_MODULE_BEGIN("security-util")
TEST_MODULE_TEST(test_mcode_encode_decode_eamuse)
TEST_MODULE_TEST(test_mcode_encode_decode_jubeat)
TEST_MODULE_TEST(test_mcode_encode_decode_reverse_eamuse)
TEST_MODULE_TEST(test_mcode_encode_decode_reverse_jubeat)
TEST_MODULE_END()
