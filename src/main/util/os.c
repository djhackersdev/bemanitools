#define LOG_MODULE "util-os"

#include <windows.h>

#include <versionhelpers.h>

#include <stdbool.h>

#include "iface-core/log.h"

#include "util/os.h"
#include "util/str.h"

#define STATUS_SUCCESS (0x00000000)

typedef LONG NTSTATUS, *PNTSTATUS;
typedef NTSTATUS(WINAPI *RtlGetVersion_t)(PRTL_OSVERSIONINFOW);

static const char *human_readable_version(DWORD major, DWORD minor)
{
    if (IsWindowsServer()) {
        if (major == 5 && minor == 2) {
            return "Windows Server 2003 (R2)";
        } else if (major == 6 && minor == 0) {
            return "Windows Server 2008";
        } else if (major == 6 && minor == 1) {
            return "Windows Server 2008 R2";
        } else if (major == 6 && minor == 2) {
            return "Windows Server 2012";
        } else if (major == 6 && minor == 3) {
            return "Windows Server 2012 R2";
        } else if (major == 10 && minor == 0) {
            return "Windows Server 2016/2019";
        } else {
            return "Unknown server version";
        }
    } else {
        if (major == 5 && minor == 0) {
            return "Windows 2000";
        } else if (major == 5 && minor == 1) {
            return "Windows XP";
        } else if (major == 5 && minor == 2) {
            return "Windows XP 64-bit";
        } else if (major == 6 && minor == 0) {
            return "Windows Vista";
        } else if (major == 6 && minor == 1) {
            return "Windows 7";
        } else if (major == 6 && minor == 2) {
            return "Windows 8";
        } else if (major == 6 && minor == 3) {
            return "Windows 8.1";
        } else if (major == 10 && minor == 0) {
            return "Windows 10";
        } else {
            return "Unknown client version";
        }
    }
}

static bool os_get_real_win_version(PRTL_OSVERSIONINFOW version)
{
    HMODULE module;
    RtlGetVersion_t rtl_get_version;

    module = GetModuleHandleA("ntdll.dll");

    if (!module) {
        return false;
    }

    rtl_get_version = (RtlGetVersion_t) GetProcAddress(module, "RtlGetVersion");

    if (!rtl_get_version) {
        return false;
    }

    return rtl_get_version(version) == STATUS_SUCCESS;
}

bool os_version_get(struct os_version *version)
{
    log_assert(version);

    RTL_OSVERSIONINFOW rovi;
    char *version_str;
    const char *readable_version;

    memset(&rovi, 0, sizeof(RTL_OSVERSIONINFOW));
    rovi.dwOSVersionInfoSize = sizeof(rovi);

    version_str = NULL;

    if (!os_get_real_win_version(&rovi)) {
        return false;
    }

    // Contains additional version info, e.g. "Service Pack 3" for XP
    wstr_narrow(rovi.szCSDVersion, &version_str);

    readable_version =
        human_readable_version(rovi.dwMajorVersion, rovi.dwMinorVersion);

    strcpy(version->name, readable_version);
    version->major = rovi.dwMajorVersion;
    version->minor = rovi.dwMinorVersion;
    version->build = rovi.dwBuildNumber;
    version->platform_id = rovi.dwPlatformId;
    strcpy(version->extension, version_str);

    if (version_str) {
        free(version_str);
    }

    return true;
}

void os_version_log()
{
    struct os_version version;

    if (!os_version_get(&version)) {
        log_warning("Could not detect OS version");
    } else {
        log_info(
            "OS version: %s - %d.%d.%d.%d - %s",
            version.name,
            version.major,
            version.minor,
            version.build,
            version.platform_id,
            version.extension);
    }
}