#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

#include "util/log.h"

struct CoinParam;
struct EEP_HISTORY;

typedef int32_t (*usb_boot_security_t)(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4);
typedef int32_t (*usb_boot_security_all_t)(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4);
typedef int32_t (*usb_boot_security_all_r_t)(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4);
typedef int32_t (*usb_check_alive_t)();
typedef int32_t (*usb_check_security_t)(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4);
typedef int32_t (*usb_check_security_eep_t)(int32_t param1);
typedef int32_t (*usb_check_security_new_t)(int32_t param1);
typedef int32_t (*usb_coin_blocker_t)(int32_t param1);
typedef int32_t (*usb_coin_get2_t)(struct CoinParam *param1);
typedef int32_t (*usb_coin_get_t)(int32_t param1);
typedef int32_t (*usb_coin_meter_down_t)(int32_t param1);
typedef int32_t (*usb_coin_meter_up_t)(int32_t param1);
typedef int32_t (*usb_coin_mode_t)(int32_t param1);
typedef int32_t (*usb_coin_up_t)(int32_t param1);
typedef int32_t (*usb_eep_read_t)();
typedef int32_t (*usb_eep_read_done_t)(uint8_t *param1);
typedef int32_t (*usb_eep_test_t)();
typedef int32_t (*usb_eep_write_t)(uint8_t *param1);
typedef int32_t (*usb_eep_write_done_t)();
typedef int32_t (*usb_end_t)();
typedef int32_t (*usb_factory_mode_init_t)(uint8_t *param1);
typedef int32_t (*usb_firm_result_t)();
typedef int32_t (*usb_get_error_t)(char *param1);
typedef int32_t (*usb_get_keyid_t)(uint8_t *param1, int32_t param2);
typedef int32_t (*usb_get_mute_t)();
typedef int32_t (*usb_get_pcbid_t)(uint8_t *param1);
typedef int32_t (*usb_get_security_t)(int32_t param1, uint8_t *param2);
typedef int32_t (*usb_is_hi_speed_t)();
typedef int32_t (*usb_lamp_t)(int32_t param1);
typedef int32_t (*usb_mute_t)(int32_t param1);
typedef int32_t (*usb_pad_read_t)(uint32_t *param1);
typedef int32_t (*usb_pad_read_last_t)(uint8_t *param1);
typedef int32_t (*usb_read_eep_history_t)(struct EEP_HISTORY *param1);
typedef int32_t (*usb_security_get_id_t)();
typedef int32_t (*usb_security_get_id_done_t)(uint8_t *param1);
typedef int32_t (*usb_security_init_t)();
typedef int32_t (*usb_security_init_done_t)();
typedef int32_t (*usb_security_read_t)();
typedef int32_t (*usb_security_read_done_t)(uint8_t *param1);
typedef int32_t (*usb_security_search_t)();
typedef int32_t (*usb_security_search_done_t)();
typedef int32_t (*usb_security_select_t)(int32_t param1);
typedef int32_t (*usb_security_select_done_t)();
typedef int32_t (*usb_security_test_t)(int32_t param1);
typedef int32_t (*usb_security_write_t)(uint8_t *param1);
typedef int32_t (*usb_security_write_done_t)();
typedef int32_t (*usb_set_ext_io_t)(int32_t param1);
typedef int32_t (*usb_setup_eeprom_t)(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4);
typedef int32_t (*usb_setup_eeprom_new_t)(
    int32_t param1,
    uint8_t *param2,
    int32_t param3,
    int32_t param4,
    int32_t param5);
typedef int32_t (*usb_setup_security_t)(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4);
typedef int32_t (*usb_setup_security_complete_t)(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4);
typedef int32_t (*usb_setup_security_complete_new_t)(
    int32_t param1,
    uint8_t *param2,
    int32_t param3,
    int32_t param4,
    int32_t param5);
typedef int32_t (*usb_setup_security_new_t)(
    int32_t param1,
    uint8_t *param2,
    int32_t param3,
    int32_t param4,
    int32_t param5);
typedef int32_t (*usb_start_t)(int32_t param1);
typedef int32_t (*usb_start_with_file_t)(char *param1);
typedef int32_t (*usb_wdt_reset_t)();
typedef int32_t (*usb_wdt_start_t)(int32_t param1);
typedef int32_t (*usb_wdt_start_done_t)();
typedef int32_t (*usb_wdt_stop_t)();

static usb_boot_security_t real_usb_boot_security;
static usb_boot_security_all_t real_usb_boot_security_all;
static usb_boot_security_all_r_t real_usb_boot_security_all_r;
static usb_check_alive_t real_usb_check_alive;
static usb_check_security_t real_usb_check_security;
static usb_check_security_eep_t real_usb_check_security_eep;
static usb_check_security_new_t real_usb_check_security_new;
static usb_coin_blocker_t real_usb_coin_blocker;
static usb_coin_get2_t real_usb_coin_get2;
static usb_coin_get_t real_usb_coin_get;
static usb_coin_meter_down_t real_usb_coin_meter_down;
static usb_coin_meter_up_t real_usb_coin_meter_up;
static usb_coin_mode_t real_usb_coin_mode;
static usb_coin_up_t real_usb_coin_up;
static usb_eep_read_t real_usb_eep_read;
static usb_eep_read_done_t real_usb_eep_read_done;
static usb_eep_test_t real_usb_eep_test;
static usb_eep_write_t real_usb_eep_write;
static usb_eep_write_done_t real_usb_eep_write_done;
static usb_end_t real_usb_end;
static usb_factory_mode_init_t real_usb_factory_mode_init;
static usb_firm_result_t real_usb_firm_result;
static usb_get_error_t real_usb_get_error;
static usb_get_keyid_t real_usb_get_keyid;
static usb_get_mute_t real_usb_get_mute;
static usb_get_pcbid_t real_usb_get_pcbid;
static usb_get_security_t real_usb_get_security;
static usb_is_hi_speed_t real_usb_is_hi_speed;
static usb_lamp_t real_usb_lamp;
static usb_mute_t real_usb_mute;
static usb_pad_read_t real_usb_pad_read;
static usb_pad_read_last_t real_usb_pad_read_last;
static usb_read_eep_history_t real_usb_read_eep_history;
static usb_security_get_id_t real_usb_security_get_id;
static usb_security_get_id_done_t real_usb_security_get_id_done;
static usb_security_init_t real_usb_security_init;
static usb_security_init_done_t real_usb_security_init_done;
static usb_security_read_t real_usb_security_read;
static usb_security_read_done_t real_usb_security_read_done;
static usb_security_search_t real_usb_security_search;
static usb_security_search_done_t real_usb_security_search_done;
static usb_security_select_t real_usb_security_select;
static usb_security_select_done_t real_usb_security_select_done;
static usb_security_test_t real_usb_security_test;
static usb_security_write_t real_usb_security_write;
static usb_security_write_done_t real_usb_security_write_done;
static usb_set_ext_io_t real_usb_set_ext_io;
static usb_setup_eeprom_t real_usb_setup_eeprom;
static usb_setup_eeprom_new_t real_usb_setup_eeprom_new;
static usb_setup_security_t real_usb_setup_security;
static usb_setup_security_complete_t real_usb_setup_security_complete;
static usb_setup_security_complete_new_t real_usb_setup_security_complete_new;
static usb_setup_security_new_t real_usb_setup_security_new;
static usb_start_t real_usb_start;
static usb_start_with_file_t real_usb_start_with_file;
static usb_wdt_reset_t real_usb_wdt_reset;
static usb_wdt_start_t real_usb_wdt_start;
static usb_wdt_start_done_t real_usb_wdt_start_done;
static usb_wdt_stop_t real_usb_wdt_stop;

static bool proxy_is_initialized = false;

void ezusb2_proxy_initialize(HMODULE pe)
{
    if (proxy_is_initialized)
        return;

    log_assert(pe != NULL);

    proxy_is_initialized = true;

    real_usb_boot_security = (usb_boot_security_t) GetProcAddress(
        pe, "?usbBootSecurity@@YAHPAEHHH@Z");
    log_assert(real_usb_boot_security);

    real_usb_boot_security_all = (usb_boot_security_all_t) GetProcAddress(
        pe, "?usbBootSecurityAll@@YAHPAEHHH@Z");
    log_assert(real_usb_boot_security_all);

    real_usb_boot_security_all_r = (usb_boot_security_all_r_t) GetProcAddress(
        pe, "?usbBootSecurityAllR@@YAHPAEHHH@Z");
    log_assert(real_usb_boot_security_all_r);

    real_usb_check_alive =
        (usb_check_alive_t) GetProcAddress(pe, "?usbCheckAlive@@YAHXZ");
    log_assert(real_usb_check_alive);

    real_usb_check_security = (usb_check_security_t) GetProcAddress(
        pe, "?usbCheckSecurity@@YAHPAEHHH@Z");
    log_assert(real_usb_check_security);

    real_usb_check_security_eep = (usb_check_security_eep_t) GetProcAddress(
        pe, "?usbCheckSecurityEep@@YAHH@Z");
    log_assert(real_usb_check_security_eep);

    real_usb_check_security_new = (usb_check_security_new_t) GetProcAddress(
        pe, "?usbCheckSecurityNew@@YAHH@Z");
    log_assert(real_usb_check_security_new);

    real_usb_coin_blocker =
        (usb_coin_blocker_t) GetProcAddress(pe, "?usbCoinBlocker@@YAHH@Z");
    log_assert(real_usb_coin_blocker);

    real_usb_coin_get2 = (usb_coin_get2_t) GetProcAddress(
        pe, "?usbCoinGet2@@YAHPAUCoinParam@@@Z");
    log_assert(real_usb_coin_get2);

    real_usb_coin_get =
        (usb_coin_get_t) GetProcAddress(pe, "?usbCoinGet@@YAHH@Z");
    log_assert(real_usb_coin_get);

    real_usb_coin_meter_down =
        (usb_coin_meter_down_t) GetProcAddress(pe, "?usbCoinMeterDown@@YAHH@Z");
    log_assert(real_usb_coin_meter_down);

    real_usb_coin_meter_up =
        (usb_coin_meter_up_t) GetProcAddress(pe, "?usbCoinMeterUp@@YAHH@Z");
    log_assert(real_usb_coin_meter_up);

    real_usb_coin_mode =
        (usb_coin_mode_t) GetProcAddress(pe, "?usbCoinMode@@YAHH@Z");
    log_assert(real_usb_coin_mode);

    real_usb_coin_up = (usb_coin_up_t) GetProcAddress(pe, "?usbCoinUp@@YAHH@Z");
    log_assert(real_usb_coin_up);

    real_usb_eep_read =
        (usb_eep_read_t) GetProcAddress(pe, "?usbEepRead@@YAHXZ");
    log_assert(real_usb_eep_read);

    real_usb_eep_read_done =
        (usb_eep_read_done_t) GetProcAddress(pe, "?usbEepReadDone@@YAHPAE@Z");
    log_assert(real_usb_eep_read_done);

    real_usb_eep_test =
        (usb_eep_test_t) GetProcAddress(pe, "?usbEepTest@@YAHXZ");
    log_assert(real_usb_eep_test);

    real_usb_eep_write =
        (usb_eep_write_t) GetProcAddress(pe, "?usbEepWrite@@YAHPAE@Z");
    log_assert(real_usb_eep_write);

    real_usb_eep_write_done =
        (usb_eep_write_done_t) GetProcAddress(pe, "?usbEepWriteDone@@YAHXZ");
    log_assert(real_usb_eep_write_done);

    real_usb_end = (usb_end_t) GetProcAddress(pe, "?usbEnd@@YAHXZ");
    log_assert(real_usb_end);

    real_usb_factory_mode_init = (usb_factory_mode_init_t) GetProcAddress(
        pe, "?usbFactoryModeInit@@YAHPAE@Z");
    log_assert(real_usb_factory_mode_init);

    real_usb_firm_result =
        (usb_firm_result_t) GetProcAddress(pe, "?usbFirmResult@@YAHXZ");
    log_assert(real_usb_firm_result);

    real_usb_get_error =
        (usb_get_error_t) GetProcAddress(pe, "?usbGetError@@YAHPAD@Z");
    log_assert(real_usb_get_error);

    real_usb_get_keyid =
        (usb_get_keyid_t) GetProcAddress(pe, "?usbGetKEYID@@YAHPAEH@Z");
    log_assert(real_usb_get_keyid);

    real_usb_get_mute =
        (usb_get_mute_t) GetProcAddress(pe, "?usbGetMute@@YAHXZ");
    log_assert(real_usb_get_mute);

    real_usb_get_pcbid =
        (usb_get_pcbid_t) GetProcAddress(pe, "?usbGetPCBID@@YAHPAE@Z");
    log_assert(real_usb_get_pcbid);

    real_usb_get_security =
        (usb_get_security_t) GetProcAddress(pe, "?usbGetSecurity@@YAHHPAE@Z");
    log_assert(real_usb_get_security);

    real_usb_is_hi_speed =
        (usb_is_hi_speed_t) GetProcAddress(pe, "?usbIsHiSpeed@@YAHXZ");
    log_assert(real_usb_is_hi_speed);

    real_usb_lamp = (usb_lamp_t) GetProcAddress(pe, "?usbLamp@@YAHH@Z");
    log_assert(real_usb_lamp);

    real_usb_mute = (usb_mute_t) GetProcAddress(pe, "?usbMute@@YAHH@Z");
    log_assert(real_usb_mute);

    real_usb_pad_read =
        (usb_pad_read_t) GetProcAddress(pe, "?usbPadRead@@YAHPAK@Z");
    log_assert(real_usb_pad_read);

    real_usb_pad_read_last =
        (usb_pad_read_last_t) GetProcAddress(pe, "?usbPadReadLast@@YAHPAE@Z");
    log_assert(real_usb_pad_read_last);

    real_usb_read_eep_history = (usb_read_eep_history_t) GetProcAddress(
        pe, "?usbReadEepHistory@@YAHPAUEEP_HISTORY@@@Z");
    log_assert(real_usb_read_eep_history);

    real_usb_security_get_id =
        (usb_security_get_id_t) GetProcAddress(pe, "?usbSecurityGetId@@YAHXZ");
    log_assert(real_usb_security_get_id);

    real_usb_security_get_id_done = (usb_security_get_id_done_t) GetProcAddress(
        pe, "?usbSecurityGetIdDone@@YAHPAE@Z");
    log_assert(real_usb_security_get_id_done);

    real_usb_security_init =
        (usb_security_init_t) GetProcAddress(pe, "?usbSecurityInit@@YAHXZ");
    log_assert(real_usb_security_init);

    real_usb_security_init_done = (usb_security_init_done_t) GetProcAddress(
        pe, "?usbSecurityInitDone@@YAHXZ");
    log_assert(real_usb_security_init_done);

    real_usb_security_read =
        (usb_security_read_t) GetProcAddress(pe, "?usbSecurityRead@@YAHXZ");
    log_assert(real_usb_security_read);

    real_usb_security_read_done = (usb_security_read_done_t) GetProcAddress(
        pe, "?usbSecurityReadDone@@YAHPAE@Z");
    log_assert(real_usb_security_read_done);

    real_usb_security_search =
        (usb_security_search_t) GetProcAddress(pe, "?usbSecuritySearch@@YAHXZ");
    log_assert(real_usb_security_search);

    real_usb_security_search_done = (usb_security_search_done_t) GetProcAddress(
        pe, "?usbSecuritySearchDone@@YAHXZ");
    log_assert(real_usb_security_search_done);

    real_usb_security_select = (usb_security_select_t) GetProcAddress(
        pe, "?usbSecuritySelect@@YAHH@Z");
    log_assert(real_usb_security_select);

    real_usb_security_select_done = (usb_security_select_done_t) GetProcAddress(
        pe, "?usbSecuritySelectDone@@YAHXZ");
    log_assert(real_usb_security_select_done);

    real_usb_security_test =
        (usb_security_test_t) GetProcAddress(pe, "?usbSecurityTest@@YAHH@Z");
    log_assert(real_usb_security_test);

    real_usb_security_write = (usb_security_write_t) GetProcAddress(
        pe, "?usbSecurityWrite@@YAHPAE@Z");
    log_assert(real_usb_security_write);

    real_usb_security_write_done = (usb_security_write_done_t) GetProcAddress(
        pe, "?usbSecurityWriteDone@@YAHXZ");
    log_assert(real_usb_security_write_done);

    real_usb_set_ext_io =
        (usb_set_ext_io_t) GetProcAddress(pe, "?usbSetExtIo@@YAHH@Z");
    log_assert(real_usb_set_ext_io);

    real_usb_setup_eeprom =
        (usb_setup_eeprom_t) GetProcAddress(pe, "?usbSetupEeprom@@YAHPAEHHH@Z");
    log_assert(real_usb_setup_eeprom);

    real_usb_setup_eeprom_new = (usb_setup_eeprom_new_t) GetProcAddress(
        pe, "?usbSetupEepromNew@@YAHHPAEHHH@Z");
    log_assert(real_usb_setup_eeprom_new);

    real_usb_setup_security = (usb_setup_security_t) GetProcAddress(
        pe, "?usbSetupSecurity@@YAHPAEHHH@Z");
    log_assert(real_usb_setup_security);

    real_usb_setup_security_complete =
        (usb_setup_security_complete_t) GetProcAddress(
            pe, "?usbSetupSecurityComplete@@YAHPAEHHH@Z");
    log_assert(real_usb_setup_security_complete);

    real_usb_setup_security_complete_new =
        (usb_setup_security_complete_new_t) GetProcAddress(
            pe, "?usbSetupSecurityCompleteNew@@YAHHPAEHHH@Z");
    log_assert(real_usb_setup_security_complete_new);

    real_usb_setup_security_new = (usb_setup_security_new_t) GetProcAddress(
        pe, "?usbSetupSecurityNew@@YAHHPAEHHH@Z");
    log_assert(real_usb_setup_security_new);

    real_usb_start = (usb_start_t) GetProcAddress(pe, "?usbStart@@YAHH@Z");
    log_assert(real_usb_start);

    real_usb_start_with_file = (usb_start_with_file_t) GetProcAddress(
        pe, "?usbStartWithFile@@YAHPAD@Z");
    log_assert(real_usb_start_with_file);

    real_usb_wdt_reset =
        (usb_wdt_reset_t) GetProcAddress(pe, "?usbWdtReset@@YAHXZ");
    log_assert(real_usb_wdt_reset);

    real_usb_wdt_start =
        (usb_wdt_start_t) GetProcAddress(pe, "?usbWdtStart@@YAHH@Z");
    log_assert(real_usb_wdt_start);

    real_usb_wdt_start_done =
        (usb_wdt_start_done_t) GetProcAddress(pe, "?usbWdtStartDone@@YAHXZ");
    log_assert(real_usb_wdt_start_done);

    real_usb_wdt_stop =
        (usb_wdt_stop_t) GetProcAddress(pe, "?usbWdtStop@@YAHXZ");
    log_assert(real_usb_wdt_stop);
}

int32_t proxy_usb_boot_security(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4)
{
    return real_usb_boot_security(param1, param2, param3, param4);
}

int32_t proxy_usb_boot_security_all(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4)
{
    return real_usb_boot_security_all(param1, param2, param3, param4);
}

int32_t proxy_usb_boot_security_all_r(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4)
{
    return real_usb_boot_security_all_r(param1, param2, param3, param4);
}

int32_t proxy_usb_check_alive()
{
    return real_usb_check_alive();
}

int32_t proxy_usb_check_security(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4)
{
    return real_usb_check_security(param1, param2, param3, param4);
}

int32_t proxy_usb_check_security_eep(int32_t param1)
{
    return real_usb_check_security_eep(param1);
}

int32_t proxy_usb_check_security_new(int32_t param1)
{
    return real_usb_check_security_new(param1);
}

int32_t proxy_usb_coin_blocker(int32_t param1)
{
    return real_usb_coin_blocker(param1);
}

int32_t proxy_usb_coin_get2(struct CoinParam *param1)
{
    return real_usb_coin_get2(param1);
}

int32_t proxy_usb_coin_get(int32_t param1)
{
    return real_usb_coin_get(param1);
}

int32_t proxy_usb_coin_meter_down(int32_t param1)
{
    return real_usb_coin_meter_down(param1);
}

int32_t proxy_usb_coin_meter_up(int32_t param1)
{
    return real_usb_coin_meter_up(param1);
}

int32_t proxy_usb_coin_mode(int32_t param1)
{
    return real_usb_coin_mode(param1);
}

int32_t proxy_usb_coin_up(int32_t param1)
{
    return real_usb_coin_up(param1);
}

int32_t proxy_usb_eep_read()
{
    return real_usb_eep_read();
}

int32_t proxy_usb_eep_read_done(uint8_t *param1)
{
    return real_usb_eep_read_done(param1);
}

int32_t proxy_usb_eep_test()
{
    return real_usb_eep_test();
}

int32_t proxy_usb_eep_write(uint8_t *param1)
{
    return real_usb_eep_write(param1);
}

int32_t proxy_usb_eep_write_done()
{
    return real_usb_eep_write_done();
}

int32_t proxy_usb_end()
{
    return real_usb_end();
}

int32_t proxy_usb_factory_mode_init(uint8_t *param1)
{
    return real_usb_factory_mode_init(param1);
}

int32_t proxy_usb_firm_result()
{
    return real_usb_firm_result();
}

int32_t proxy_usb_get_error(char *param1)
{
    return real_usb_get_error(param1);
}

int32_t proxy_usb_get_keyid(uint8_t *param1, int32_t param2)
{
    return real_usb_get_keyid(param1, param2);
}

int32_t proxy_usb_get_mute()
{
    return real_usb_get_mute();
}

int32_t proxy_usb_get_pcbid(uint8_t *param1)
{
    return real_usb_get_pcbid(param1);
}

int32_t proxy_usb_get_security(int32_t param1, uint8_t *param2)
{
    return real_usb_get_security(param1, param2);
}

int32_t proxy_usb_is_hi_speed()
{
    return real_usb_is_hi_speed();
}

int32_t proxy_usb_lamp(int32_t param1)
{
    return real_usb_lamp(param1);
}

int32_t proxy_usb_mute(int32_t param1)
{
    return real_usb_mute(param1);
}

int32_t proxy_usb_pad_read(uint32_t *param1)
{
    return real_usb_pad_read(param1);
}

int32_t proxy_usb_pad_read_last(uint8_t *param1)
{
    return real_usb_pad_read_last(param1);
}

int32_t proxy_usb_read_eep_history(struct EEP_HISTORY *param1)
{
    return real_usb_read_eep_history(param1);
}

int32_t proxy_usb_security_get_id()
{
    return real_usb_security_get_id();
}

int32_t proxy_usb_security_get_id_done(uint8_t *param1)
{
    return real_usb_security_get_id_done(param1);
}

int32_t proxy_usb_security_init()
{
    return real_usb_security_init();
}

int32_t proxy_usb_security_init_done()
{
    return real_usb_security_init_done();
}

int32_t proxy_usb_security_read()
{
    return real_usb_security_read();
}

int32_t proxy_usb_security_read_done(uint8_t *param1)
{
    return real_usb_security_read_done(param1);
}

int32_t proxy_usb_security_search()
{
    return real_usb_security_search();
}

int32_t proxy_usb_security_search_done()
{
    return real_usb_security_search_done();
}

int32_t proxy_usb_security_select(int32_t param1)
{
    return real_usb_security_select(param1);
}

int32_t proxy_usb_security_select_done()
{
    return real_usb_security_select_done();
}

int32_t proxy_usb_security_test(int32_t param1)
{
    return real_usb_security_test(param1);
}

int32_t proxy_usb_security_write(uint8_t *param1)
{
    return real_usb_security_write(param1);
}

int32_t proxy_usb_security_write_done()
{
    return real_usb_security_write_done();
}

int32_t proxy_usb_set_ext_io(int32_t param1)
{
    return real_usb_set_ext_io(param1);
}

int32_t proxy_usb_setup_eeprom(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4)
{
    return real_usb_setup_eeprom(param1, param2, param3, param4);
}

int32_t proxy_usb_setup_eeprom_new(
    int32_t param1,
    uint8_t *param2,
    int32_t param3,
    int32_t param4,
    int32_t param5)
{
    return real_usb_setup_eeprom_new(param1, param2, param3, param4, param5);
}

int32_t proxy_usb_setup_security(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4)
{
    return real_usb_setup_security(param1, param2, param3, param4);
}

int32_t proxy_usb_setup_security_complete(
    uint8_t *param1, int32_t param2, int32_t param3, int32_t param4)
{
    return real_usb_setup_security_complete(param1, param2, param3, param4);
}

int32_t proxy_usb_setup_security_complete_new(
    int32_t param1,
    uint8_t *param2,
    int32_t param3,
    int32_t param4,
    int32_t param5)
{
    return real_usb_setup_security_complete_new(
        param1, param2, param3, param4, param5);
}

int32_t proxy_usb_setup_security_new(
    int32_t param1,
    uint8_t *param2,
    int32_t param3,
    int32_t param4,
    int32_t param5)
{
    return real_usb_setup_security_new(param1, param2, param3, param4, param5);
}

int32_t proxy_usb_start(int32_t param1)
{
    return real_usb_start(param1);
}

int32_t proxy_usb_start_with_file(char *param1)
{
    return real_usb_start_with_file(param1);
}

int32_t proxy_usb_wdt_reset()
{
    return real_usb_wdt_reset();
}

int32_t proxy_usb_wdt_start(int32_t param1)
{
    return real_usb_wdt_start(param1);
}

int32_t proxy_usb_wdt_start_done()
{
    return real_usb_wdt_start_done();
}

int32_t proxy_usb_wdt_stop()
{
    return real_usb_wdt_stop();
}
