#define LOG_MODULE "asio-reghook"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <initguid.h>
// clang-format on

#include <winreg.h>

#include <stdio.h>

#include "asio/asio-reghook.h"

#include "core/log.h"

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "util/defs.h"
#include "util/str.h"
#include "util/time.h"

// MAXDRVNAMELEN
#define ASIO_NAME_STRING_SIZE 128

static char asio_expected_name[ASIO_NAME_STRING_SIZE] = "XONAR SOUND CARD(64)";
static char asio_target_name[ASIO_NAME_STRING_SIZE] = "XONAR SOUND CARD(64)";
static HKEY asio_root_key;
static HKEY asio_sub_key;

LSTATUS my_RegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);

LSTATUS my_RegEnumKeyA(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName);

LSTATUS my_RegOpenKeyExA(
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult);

LSTATUS my_RegQueryValueExA(
    HKEY hKey,
    LPCSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData);

LSTATUS my_RegCloseKey(HKEY hKey);

LSTATUS (*real_RegOpenKeyA)(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);

LSTATUS(*real_RegEnumKeyA)
(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName);

LSTATUS(*real_RegOpenKeyExA)
(HKEY hKey,
 LPCSTR lpSubKey,
 DWORD ulOptions,
 REGSAM samDesired,
 PHKEY phkResult);

LSTATUS(*real_RegQueryValueExA)
(HKEY hKey,
 LPCSTR lpValueName,
 LPDWORD lpReserved,
 LPDWORD lpType,
 LPBYTE lpData,
 LPDWORD lpcbData);

LSTATUS (*real_RegCloseKey)(HKEY hKey);

static const struct hook_symbol asio_reghook_advapi32_syms[] = {
    {.name = "RegOpenKeyA",
     .patch = my_RegOpenKeyA,
     .link = (void **) &real_RegOpenKeyA},
    {.name = "RegEnumKeyA",
     .patch = my_RegEnumKeyA,
     .link = (void **) &real_RegEnumKeyA},
    {.name = "RegOpenKeyExA",
     .patch = my_RegOpenKeyExA,
     .link = (void **) &real_RegOpenKeyExA},
    {.name = "RegQueryValueExA",
     .patch = my_RegQueryValueExA,
     .link = (void **) &real_RegQueryValueExA},
    {.name = "RegCloseKey",
     .patch = my_RegCloseKey,
     .link = (void **) &real_RegCloseKey},
};

LSTATUS my_RegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
    LSTATUS result = real_RegOpenKeyA(hKey, lpSubKey, phkResult);

    if (!strcmp("software\\asio", lpSubKey)) {
        log_info("ASIO root key found");
        asio_root_key = *phkResult;
    }

    return result;
}

LSTATUS my_RegEnumKeyA(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName)
{
    LSTATUS result = real_RegEnumKeyA(hKey, dwIndex, lpName, cchName);

    if (hKey == asio_root_key) {
        if (!strcmp(asio_target_name, lpName)) {
            log_info(
                "Replacing key [%s] with [%s]",
                asio_target_name,
                asio_expected_name);
            strncpy(lpName, asio_expected_name, cchName);
        }
    }

    return result;
}

LSTATUS my_RegOpenKeyExA(
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult)
{
    if (hKey == asio_root_key) {
        if (!strcmp(asio_expected_name, lpSubKey)) {
            // swap to the real one so the call succeeds
            LSTATUS result = real_RegOpenKeyExA(
                hKey, asio_target_name, ulOptions, samDesired, phkResult);

            asio_sub_key = *phkResult;
            log_info("ASIO device key found");

            return result;
        }
    }

    return real_RegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

LSTATUS my_RegQueryValueExA(
    HKEY hKey,
    LPCSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData)
{
    DWORD lpcbData_orig = *lpcbData;
    LSTATUS result = real_RegQueryValueExA(
        hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    if (result == ERROR_SUCCESS) {
        if (hKey == asio_sub_key) {
            if (!strcmp("description", lpValueName)) {
                log_info(
                    "Replacing description [%s] with [%s]",
                    lpData,
                    asio_expected_name);
                strncpy((char *) lpData, asio_expected_name, lpcbData_orig);
            }
        }
    }

    return result;
}

LSTATUS my_RegCloseKey(HKEY hKey)
{
    if (hKey == asio_root_key) {
        asio_root_key = NULL;
    }

    if (hKey == asio_sub_key) {
        asio_sub_key = NULL;
    }

    return real_RegCloseKey(hKey);
}

void asio_reghook_init(const char *expected_name, const char *target_name)
{
    strncpy(asio_expected_name, expected_name, ASIO_NAME_STRING_SIZE - 1);
    strncpy(asio_target_name, target_name, ASIO_NAME_STRING_SIZE - 1);

    asio_expected_name[ASIO_NAME_STRING_SIZE - 1] = '\0';
    asio_target_name[ASIO_NAME_STRING_SIZE - 1] = '\0';

    hook_table_apply(
        NULL,
        "Advapi32.dll",
        asio_reghook_advapi32_syms,
        lengthof(asio_reghook_advapi32_syms));

    log_info("Inserted reg hooks for ASIO override");
}
