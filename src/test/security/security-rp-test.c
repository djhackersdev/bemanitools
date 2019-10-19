#include "security/id.h"
#include "security/rp.h"

#include "test/check.h"
#include "test/test.h"

static const struct security_mcode boot_version_iidx_09_to_13 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_E,
    .game = SECURITY_MCODE_GAME_IIDX_9,
    .region = SECURITY_MCODE_FIELD_BLANK,
    .cabinet = SECURITY_MCODE_FIELD_BLANK,
    .revision = SECURITY_MCODE_FIELD_BLANK,
};

static const uint32_t boot_seeds_iidx_09[3] = {0, 0, 0};
static const uint32_t boot_seeds_iidx_10[3] = {0, 1, 1};
static const uint32_t boot_seeds_iidx_11[3] = {0, 2, 2};
static const uint32_t boot_seeds_iidx_12[3] = {0, 3, 3};
static const uint32_t boot_seeds_iidx_13[3] = {0, 4, 4};

static const struct security_mcode black_plug_mcode_iidx_09 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_9,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static const struct security_mcode black_plug_mcode_iidx_10 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_10,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static const struct security_mcode black_plug_mcode_iidx_11 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_11,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static const struct security_mcode black_plug_mcode_iidx_12 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_12,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static const struct security_mcode black_plug_mcode_iidx_13 = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_Q,
    .game = SECURITY_MCODE_GAME_IIDX_13,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_A,
};

static const struct security_rp_eeprom exp_iidx_09_black_eeprom = {
    .signature = {0x07, 0x5D, 0x45, 0xCB, 0xDA, 0x19},
    .packed_payload = {0x86, 0x1A, 0x92, 0x42, 0x3C, 0x67},
};

static const struct security_rp_eeprom exp_iidx_10_black_eeprom = {
    .signature = {0x10, 0x45, 0x30, 0x7D, 0x1C, 0x13},
    .packed_payload = {0x86, 0x1A, 0x91, 0x42, 0x4C, 0x67},
};

static const struct security_rp_eeprom exp_iidx_11_black_eeprom = {
    .signature = {0xA2, 0x8B, 0x97, 0xA3, 0x3B, 0x23},
    .packed_payload = {0x86, 0x1A, 0x91, 0x46, 0x5C, 0x67},
};

static const struct security_rp_eeprom exp_iidx_12_black_eeprom = {
    .signature = {0x1B, 0xC5, 0xF1, 0x02, 0xF0, 0xB8},
    .packed_payload = {0x86, 0x1A, 0xAF, 0x8E, 0x5C, 0x67},
};

static const struct security_rp_eeprom exp_iidx_13_black_eeprom = {
    .signature = {0x37, 0xCE, 0x49, 0x54, 0x20, 0x83},
    .packed_payload = {0x86, 0x1A, 0xA4, 0x92, 0x6C, 0x67},
};

static void test_encode_iidx_09_black_dongle()
{
    struct security_rp_eeprom result;

    security_rp_generate_signed_eeprom_data(
        &boot_version_iidx_09_to_13,
        boot_seeds_iidx_09,
        &black_plug_mcode_iidx_09,
        &security_id_default,
        &result);

    check_data_eq(
        (void *) &exp_iidx_09_black_eeprom,
        sizeof(struct security_rp_eeprom),
        (void *) &result,
        sizeof(struct security_rp_eeprom));
}

static void test_encode_iidx_10_black_dongle()
{
    struct security_rp_eeprom result;

    security_rp_generate_signed_eeprom_data(
        &boot_version_iidx_09_to_13,
        boot_seeds_iidx_10,
        &black_plug_mcode_iidx_10,
        &security_id_default,
        &result);

    check_data_eq(
        (void *) &exp_iidx_10_black_eeprom,
        sizeof(struct security_rp_eeprom),
        (void *) &result,
        sizeof(struct security_rp_eeprom));
}

static void test_encode_iidx_11_black_dongle()
{
    struct security_rp_eeprom result;

    security_rp_generate_signed_eeprom_data(
        &boot_version_iidx_09_to_13,
        boot_seeds_iidx_11,
        &black_plug_mcode_iidx_11,
        &security_id_default,
        &result);

    check_data_eq(
        (void *) &exp_iidx_11_black_eeprom,
        sizeof(struct security_rp_eeprom),
        (void *) &result,
        sizeof(struct security_rp_eeprom));
}

static void test_encode_iidx_12_black_dongle()
{
    struct security_rp_eeprom result;

    security_rp_generate_signed_eeprom_data(
        &boot_version_iidx_09_to_13,
        boot_seeds_iidx_12,
        &black_plug_mcode_iidx_12,
        &security_id_default,
        &result);

    check_data_eq(
        (void *) &exp_iidx_12_black_eeprom,
        sizeof(struct security_rp_eeprom),
        (void *) &result,
        sizeof(struct security_rp_eeprom));
}

static void test_encode_iidx_13_black_dongle()
{
    struct security_rp_eeprom result;

    security_rp_generate_signed_eeprom_data(
        &boot_version_iidx_09_to_13,
        boot_seeds_iidx_13,
        &black_plug_mcode_iidx_13,
        &security_id_default,
        &result);

    check_data_eq(
        (void *) &exp_iidx_13_black_eeprom,
        sizeof(struct security_rp_eeprom),
        (void *) &result,
        sizeof(struct security_rp_eeprom));
}

TEST_MODULE_BEGIN("security-rp")
TEST_MODULE_TEST(test_encode_iidx_09_black_dongle)
TEST_MODULE_TEST(test_encode_iidx_10_black_dongle)
TEST_MODULE_TEST(test_encode_iidx_11_black_dongle)
TEST_MODULE_TEST(test_encode_iidx_12_black_dongle)
TEST_MODULE_TEST(test_encode_iidx_13_black_dongle)
TEST_MODULE_END()
