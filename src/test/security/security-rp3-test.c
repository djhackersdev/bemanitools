#include "security/rp-sign-key.h"
#include "security/rp3.h"

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

static const struct security_rp3_eeprom exp_jubeat_black_eeprom = {
    .signature = {0x73, 0x66, 0xBA, 0xBD, 0xDE, 0x36},
    .packed_payload = {0xE7, 0x88, 0x52, 0x94, 0x1A, 0x8A},
    .zeros = {0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00},
    .crc = 0xA3,
};

static const struct security_rp3_eeprom exp_jubeat_white_eeprom = {
    .signature = {0x43, 0x17, 0xB4, 0x2A, 0x3E, 0x87},
    .packed_payload = {0x20, 0x08, 0x82, 0x20, 0x08, 0x82},
    .zeros = {0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00,
              0x00},
    .crc = 0x94,
};

static void test_mcode_encode_black_dongle_jubeat()
{
    struct security_rp3_eeprom result;

    security_rp3_generate_signed_eeprom_data(
        SECURITY_RP_UTIL_RP_TYPE_BLACK,
        &security_rp_sign_key_black_gfdmv4,
        &black_plug_mcode_jubeat,
        &security_id_default,
        &result);

    check_data_eq(
        (void *) &exp_jubeat_black_eeprom,
        sizeof(struct security_rp3_eeprom),
        (void *) &result,
        sizeof(struct security_rp3_eeprom));
}

static void test_mcode_encode_white_dongle_jubeat()
{
    struct security_rp3_eeprom result;

    security_rp3_generate_signed_eeprom_data(
        SECURITY_RP_UTIL_RP_TYPE_WHITE,
        &security_rp_sign_key_white_eamuse,
        &security_mcode_eamuse,
        &security_id_default,
        &result);

    check_data_eq(
        (void *) &exp_jubeat_white_eeprom,
        sizeof(struct security_rp3_eeprom),
        (void *) &result,
        sizeof(struct security_rp3_eeprom));
}

TEST_MODULE_BEGIN("security-rp3")
TEST_MODULE_TEST(test_mcode_encode_black_dongle_jubeat)
TEST_MODULE_TEST(test_mcode_encode_white_dongle_jubeat)
TEST_MODULE_END()
