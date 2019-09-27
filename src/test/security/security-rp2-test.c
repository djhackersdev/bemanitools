#include "security/id.h"
#include "security/rp2.h"

#include "test/check.h"
#include "test/test.h"

static const struct security_mcode boot_version_iidx_14 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_14,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static const struct security_mcode boot_version_iidx_15 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_15,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static const struct security_mcode boot_version_iidx_16 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_16,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static const struct security_mcode boot_version_iidx_17 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_C,
    .game = SECURITY_MCODE_GAME_IIDX_17,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static const struct security_mcode black_plug_mcode_iidx_14 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_14,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_FIELD_NULL,
};

static const struct security_mcode black_plug_mcode_iidx_15 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_15,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_FIELD_NULL,
};

static const struct security_mcode black_plug_mcode_iidx_16 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_16,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_FIELD_NULL,
};

static const struct security_mcode black_plug_mcode_iidx_17 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_C,
    .game = SECURITY_MCODE_GAME_IIDX_17,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_FIELD_NULL,
};

static const struct security_rp2_eeprom exp_iidx_14_black_eeprom = {
    .signature = {0x17, 0xC6, 0x2B, 0x6A, 0xDA, 0x29},
    .packed_payload = {0x67, 0x7C, 0xB2, 0xA4, 0x1A, 0x82},
};

static const struct security_rp2_eeprom exp_iidx_15_black_eeprom = {
    .signature = {0xCE, 0xF8, 0xC7, 0xEF, 0xA6, 0xDF},
    .packed_payload = {0x67, 0x8C, 0x92, 0xA4, 0x1A, 0x82},
};

static const struct security_rp2_eeprom exp_iidx_16_black_eeprom = {
    .signature = {0xE0, 0x58, 0xDA, 0xE4, 0x61, 0xF9},
    .packed_payload = {0x67, 0x9C, 0x42, 0x90, 0x1A, 0x82},
};

static const struct security_rp2_eeprom exp_iidx_17_black_eeprom = {
    .signature = {0xC3, 0x4B, 0xF8, 0xAA, 0xC1, 0x3E},
    .packed_payload = {0xE7, 0xA8, 0x92, 0xAA, 0x1A, 0x82},
};

static const struct security_rp2_eeprom exp_iidx_14_white_eeprom = {
    .signature = {0x45, 0xF7, 0xE3, 0x4C, 0x59, 0xE0},
    .packed_payload = {0x20, 0x08, 0x82, 0x20, 0x08, 0x82},
};

static const struct security_rp2_eeprom exp_iidx_15_white_eeprom = {
    .signature = {0x45, 0xF7, 0xE3, 0x4C, 0x59, 0xE0},
    .packed_payload = {0x20, 0x08, 0x82, 0x20, 0x08, 0x82},
};

static const struct security_rp2_eeprom exp_iidx_16_white_eeprom = {
    .signature = {0x45, 0xF7, 0xE3, 0x4C, 0x59, 0xE0},
    .packed_payload = {0x20, 0x08, 0x82, 0x20, 0x08, 0x82},
};

static const struct security_rp2_eeprom exp_iidx_17_white_eeprom = {
    .signature = {0x45, 0xF7, 0xE3, 0x4C, 0x59, 0xE0},
    .packed_payload = {0x20, 0x08, 0x82, 0x20, 0x08, 0x82},
};

static void test_encode_iidx_14_black_dongle()
{
    struct security_rp2_eeprom result;

    security_rp2_generate_signed_eeprom_data(SECURITY_RP_UTIL_RP_TYPE_BLACK, 
        &boot_version_iidx_14, &black_plug_mcode_iidx_14, &security_id_default,
        &result);

    check_data_eq((void*) &result, sizeof(struct security_rp2_eeprom),
        (void*) &exp_iidx_14_black_eeprom, sizeof(struct security_rp2_eeprom));
}

static void test_encode_iidx_14_white_dongle()
{
    struct security_rp2_eeprom result;

    security_rp2_generate_signed_eeprom_data(SECURITY_RP_UTIL_RP_TYPE_WHITE, 
        &boot_version_iidx_14, &security_mcode_eamuse, &security_id_default,
        &result);

    check_data_eq((void*) &result, sizeof(struct security_rp2_eeprom),
        (void*) &exp_iidx_14_white_eeprom, sizeof(struct security_rp2_eeprom));
}

static void test_encode_iidx_15_black_dongle()
{
    struct security_rp2_eeprom result;

    security_rp2_generate_signed_eeprom_data(SECURITY_RP_UTIL_RP_TYPE_BLACK, 
        &boot_version_iidx_15, &black_plug_mcode_iidx_15, &security_id_default,
        &result);

    check_data_eq((void*) &result, sizeof(struct security_rp2_eeprom),
        (void*) &exp_iidx_15_black_eeprom, sizeof(struct security_rp2_eeprom));
}

static void test_encode_iidx_15_white_dongle()
{
    struct security_rp2_eeprom result;

    security_rp2_generate_signed_eeprom_data(SECURITY_RP_UTIL_RP_TYPE_WHITE, 
        &boot_version_iidx_15, &security_mcode_eamuse, &security_id_default,
        &result);

    check_data_eq((void*) &result, sizeof(struct security_rp2_eeprom),
        (void*) &exp_iidx_15_white_eeprom, sizeof(struct security_rp2_eeprom));
}

static void test_encode_iidx_16_black_dongle()
{
    struct security_rp2_eeprom result;

    security_rp2_generate_signed_eeprom_data(SECURITY_RP_UTIL_RP_TYPE_BLACK, 
        &boot_version_iidx_16, &black_plug_mcode_iidx_16, &security_id_default,
        &result);

    check_data_eq((void*) &result, sizeof(struct security_rp2_eeprom),
        (void*) &exp_iidx_16_black_eeprom, sizeof(struct security_rp2_eeprom));
}

static void test_encode_iidx_16_white_dongle()
{
    struct security_rp2_eeprom result;

    security_rp2_generate_signed_eeprom_data(SECURITY_RP_UTIL_RP_TYPE_WHITE, 
        &boot_version_iidx_16, &security_mcode_eamuse, &security_id_default,
        &result);

    check_data_eq((void*) &result, sizeof(struct security_rp2_eeprom),
        (void*) &exp_iidx_16_white_eeprom, sizeof(struct security_rp2_eeprom));
}

static void test_encode_iidx_17_black_dongle()
{
    struct security_rp2_eeprom result;

    security_rp2_generate_signed_eeprom_data(SECURITY_RP_UTIL_RP_TYPE_BLACK, 
        &boot_version_iidx_17, &black_plug_mcode_iidx_17, &security_id_default,
        &result);

    check_data_eq((void*) &result, sizeof(struct security_rp2_eeprom),
        (void*) &exp_iidx_17_black_eeprom, sizeof(struct security_rp2_eeprom));
}

static void test_encode_iidx_17_white_dongle()
{
    struct security_rp2_eeprom result;

    security_rp2_generate_signed_eeprom_data(SECURITY_RP_UTIL_RP_TYPE_WHITE, 
        &boot_version_iidx_17, &security_mcode_eamuse, &security_id_default,
        &result);

    check_data_eq((void*) &result, sizeof(struct security_rp2_eeprom),
        (void*) &exp_iidx_17_white_eeprom, sizeof(struct security_rp2_eeprom));
}

TEST_MODULE_BEGIN("security-rp2")
TEST_MODULE_TEST(test_encode_iidx_14_black_dongle)
TEST_MODULE_TEST(test_encode_iidx_14_white_dongle)
TEST_MODULE_TEST(test_encode_iidx_15_black_dongle)
TEST_MODULE_TEST(test_encode_iidx_15_white_dongle)
TEST_MODULE_TEST(test_encode_iidx_16_black_dongle)
TEST_MODULE_TEST(test_encode_iidx_16_white_dongle)
TEST_MODULE_TEST(test_encode_iidx_17_black_dongle)
TEST_MODULE_TEST(test_encode_iidx_17_white_dongle)
TEST_MODULE_END()
