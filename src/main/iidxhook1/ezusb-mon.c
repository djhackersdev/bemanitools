#define LOG_MODULE "ezusb-mon"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/table.h"

#include "iidxhook1/ezusb-mon.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"

/* ------------------------------------------------------------------------- */

static int my_usbStart(int val);
static int my_usbSecurityInit();
static int my_usbSecuritySelectDone();
static int my_usbGetPCBID(char *buffer);
static int my_usbGetSecurity(char *buffer);
static int my_usbGetSecurityKey(char *buffer);
static int
my_usbBootSecurity(const char *boot_code, int seed_1, int seed_2, int seed_3);
static int my_usbSetupSecurityComplete(int val1, int val2, int val3, int val4);
static int my_usbMute(int val);
static int my_usbFirmResult();

static int (*real_usbStart)(int val);
static int (*real_usbSecurityInit)();
static int (*real_usbSecuritySelectDone)();
static int (*real_usbGetPCBID)(char *buffer);
static int (*real_usbGetSecurity)(char *buffer);
static int (*real_usbGetSecurityKey)(char *buffer);
static int (*real_usbBootSecurity)(
    const char *boot_code, int seed_1, int seed_2, int seed_3);
static int (*real_usbSetupSecurityComplete)(
    int val1, int val2, int val3, int val4);
static int (*real_usbMute)(int val);
static int (*real_usbFirmResult)();

/* ------------------------------------------------------------------------- */

static const struct hook_symbol ezusb_mon_hook_syms[] = {
    {.name = "usbStart",
     .patch = my_usbStart,
     .link = (void **) &real_usbStart},
    {.name = "usbSecurityInit",
     .patch = my_usbSecurityInit,
     .link = (void **) &real_usbSecurityInit},
    {.name = "usbSecuritySelectDone",
     .patch = my_usbSecuritySelectDone,
     .link = (void **) &real_usbSecuritySelectDone},
    {.name = "usbGetPCBID",
     .patch = my_usbGetPCBID,
     .link = (void **) &real_usbGetPCBID},
    {.name = "usbGetSecurity",
     .patch = my_usbGetSecurity,
     .link = (void **) &real_usbGetSecurity},
    {.name = "usbGetSecurity",
     .patch = my_usbGetSecurity,
     .link = (void **) &real_usbGetSecurity},
    {.name = "usbGetSecurityKey",
     .patch = my_usbGetSecurityKey,
     .link = (void **) &real_usbGetSecurityKey},
    {.name = "usbBootSecurity",
     .patch = my_usbBootSecurity,
     .link = (void **) &real_usbBootSecurity},
    {.name = "usbSetupSecurityComplete",
     .patch = my_usbSetupSecurityComplete,
     .link = (void **) &real_usbSetupSecurityComplete},
    {.name = "usbMute", .patch = my_usbMute, .link = (void **) &real_usbMute},
    {.name = "usbFirmResult",
     .patch = my_usbFirmResult,
     .link = (void **) &real_usbFirmResult},
};

/* ------------------------------------------------------------------------- */

static int my_usbStart(int val)
{
    int res;

    log_misc("BEFORE usbStart, val %d", val);

    res = real_usbStart(val);

    log_misc("AFTER usbStart, res: %d", res);

    return res;
}

static int my_usbSecurityInit()
{
    int res;

    log_misc("BEFORE usbSecurityInit");

    res = real_usbSecurityInit();

    log_misc("AFTER usbSecurityInit, res: %d", res);

    return res;
}

static int my_usbSecuritySelectDone()
{
    int res;

    log_misc("BEFORE usbSecuritySelectDone");

    res = real_usbSecuritySelectDone();

    log_misc("AFTER usbSecuritySelectDone, res: %d", res);

    return res;
}

static int my_usbGetPCBID(char *buffer)
{
    int res;

    log_misc("BEFORE usbGetPCBID, buffer %s", buffer);

    res = real_usbGetPCBID(buffer);

    log_misc("AFTER usbGetPCBID, res: %d, buffer %s", res, buffer);

    return res;
}

static int my_usbGetSecurity(char *buffer)
{
    int res;

    log_misc("BEFORE usbGetSecurity, buffer %s", buffer);

    res = real_usbGetSecurity(buffer);

    log_misc("AFTER usbGetSecurity, res: %d, buffer %s", res, buffer);

    return res;
}

static int my_usbGetSecurityKey(char *buffer)
{
    int res;

    log_misc("BEFORE usbGetSecurityKey, buffer %s", buffer);

    res = real_usbGetSecurityKey(buffer);

    log_misc("AFTER usbGetSecurityKey, res: %d, buffer %s", res, buffer);

    return res;
}

static int
my_usbBootSecurity(const char *boot_code, int seed_1, int seed_2, int seed_3)
{
    int res;

    log_misc(
        "BEFORE usbBootSecurity, boot_code %s, seed_1 %d, seed_2 %d, seed_3 %d",
        boot_code,
        seed_1,
        seed_2,
        seed_3);

    res = real_usbBootSecurity(boot_code, seed_1, seed_2, seed_3);

    log_misc("AFTER usbBootSecurity, res: %d", res);

    return res;
}

static int my_usbSetupSecurityComplete(int val1, int val2, int val3, int val4)
{
    int res;

    log_misc(
        "BEFORE usbSetupSecurityComplete, val1 %d, val2 %d, val3 %d, val4 %d",
        val1,
        val2,
        val3,
        val4);

    res = real_usbSetupSecurityComplete(val1, val2, val3, val4);

    log_misc("AFTER usbSetupSecurityComplete, res: %d", res);

    return res;
}

static int my_usbMute(int val)
{
    int res;

    log_misc("BEFORE usbMute, val %d", val);

    res = real_usbMute(val);

    log_misc("AFTER usbMute, res: %d", res);

    return res;
}

static int my_usbFirmResult()
{
    int res;

    res = real_usbFirmResult();

    // Avoid log flood with constant polling of this API function as long as it
    // keeps returning 96
    if (res != 96) {
        log_misc("AFTER usbFirmResult, res (!= 96): %d", res);
    }

    return res;
}

/* ------------------------------------------------------------------------- */

void ezusb_mon_hook_init(void)
{
    hook_table_apply(
        NULL, "ezusb.dll", ezusb_mon_hook_syms, lengthof(ezusb_mon_hook_syms));

    log_info("Inserted ezusb mon hooks");
}
