#define LOG_MODULE "ezusb-proxy"

#include <windows.h>

#include <dbghelp.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ezusb-proxy/ezusb-proxy.h"

#include "util/log.h"

static usbCheckAlive_t real_usbCheckAlive;
static usbCheckSecurityNew_t real_usbCheckSecurityNew;
static usbCoinGet_t real_usbCoinGet;
static usbCoinMode_t real_usbCoinMode;
static usbEnd_t real_usbEnd;
static usbFirmResult_t real_usbFirmResult;
static usbGetKEYID_t real_usbGetKEYID;
static usbGetSecurity_t real_usbGetSecurity;
static usbLamp_t real_usbLamp;
static usbPadRead_t real_usbPadRead;
static usbPadReadLast_t real_usbPadReadLast;
static usbSecurityInit_t real_usbSecurityInit;
static usbSecurityInitDone_t real_usbSecurityInitDone;
static usbSecuritySearch_t real_usbSecuritySearch;
static usbSecuritySearchDone_t real_usbSecuritySearchDone;
static usbSecuritySelect_t real_usbSecuritySelect;
static usbSecuritySelectDone_t real_usbSecuritySelectDone;
static usbSetExtIo_t real_usbSetExtIo;
static usbStart_t real_usbStart;
static usbWdtReset_t real_usbWdtReset;
static usbWdtStart_t real_usbWdtStart;
static usbWdtStartDone_t real_usbWdtStartDone;

static FILE* log_file;
static bool real_ezusb_initialized;

static void init_real_ezusb();

int usbCheckAlive()
{
    log_misc("usbCheckAlive\n");

    return real_usbCheckAlive();
}

int usbCheckSecurityNew()
{
    log_misc("usbCheckSecurityNew\n");

    return real_usbCheckSecurityNew();
}

int usbCoinGet()
{
    log_misc("usbCoinGet\n");

    return real_usbCoinGet();
}

int usbCoinMode()
{
    log_misc("usbCoinMode\n");

    return real_usbCoinMode();
}

int usbEnd()
{
    log_misc("usbEnd\n");

    return real_usbEnd();
}

int usbFirmResult()
{
    log_misc("usbFirmResult\n");

    return real_usbFirmResult();
}

int usbGetKEYID()
{
    log_misc("usbGetKEYID\n");

    return real_usbGetKEYID();
}

int usbGetSecurity()
{
    log_misc("usbGetSecurity\n");

    return real_usbGetSecurity();
}

int usbLamp(uint32_t lamp_bits)
{
    log_misc("usbLamp\n");

    return real_usbLamp(lamp_bits);
}

int usbPadRead(unsigned int *pad_bits)
{
    log_misc("usbPadRead\n");

    *pad_bits = 0;

    return real_usbPadRead(pad_bits);
}

int usbPadReadLast(uint8_t *a1)
{
    log_misc("usbPadReadLast\n");

    memset(a1, 0, 40);

    return real_usbPadReadLast(a1);
}

int usbSecurityInit()
{
    log_misc("usbSecurityInit\n");

    return real_usbSecurityInit();
}

int usbSecurityInitDone()
{
    log_misc("usbSecurityInitDone\n");

    return real_usbSecurityInitDone();
}

int usbSecuritySearch()
{
    log_misc("usbSecuritySearch\n");

    return real_usbSecuritySearch();
}

int usbSecuritySearchDone()
{
    log_misc("usbSecuritySearchDone\n");

    return real_usbSecuritySearchDone();
}

int usbSecuritySelect()
{
    log_misc("usbSecuritySelect\n");

    return real_usbSecuritySelect();
}

int usbSecuritySelectDone()
{
    log_misc("usbSecuritySelectDone\n");

    return real_usbSecuritySelectDone();
}

int usbSetExtIo()
{
    log_misc("usbSetExtIo\n");

    return real_usbSetExtIo();
}

int usbStart()
{
    log_misc("usbStart\n");

    // usbStart is the first call by popn music to the ezusb API
    // trigger late bootstrappimg of the real ezusb
    // this setup allows any hook modules to setup hooks to any system
    // calls that ezusb is using, e.g. for emulation
    init_real_ezusb();

    return real_usbStart();
}

int usbWdtReset()
{
    log_misc("usbWdtReset\n");

    return real_usbWdtReset();
}

int usbWdtStart(int a1)
{
    log_misc("usbWdtStart\n");

    return real_usbWdtStart(a1);
}

int usbWdtStartDone()
{
    log_misc("usbWdtStartDone\n");

    return real_usbWdtStartDone();
}

static void* get_proc_address(HMODULE module, const char* name)
{
    void* addr;

    log_misc("Resolving %s...", name);

    addr = GetProcAddress(module, name);

    // TODO error reporting needs to be improved regarding visibility
    if (!addr) {
        log_fatal("Getting function address %s in ezusb-orig.dll failed: %08lX", name, GetLastError());
    }

    return addr;
}

static DllEntryPoint_t get_dll_main_address(HMODULE module)
{
    PIMAGE_NT_HEADERS header = ImageNtHeader(module);

    return (DllEntryPoint_t)(header->OptionalHeader.AddressOfEntryPoint + (DWORD_PTR) module);
}

static void init_real_ezusb()
{
    if (!real_ezusb_initialized) {
        real_ezusb_initialized = true;

        HMODULE module;
        DllEntryPoint_t dll_main;
        char buffer[MAX_PATH];

        log_info("Loading real ezusb library");

        GetCurrentDirectoryA(MAX_PATH, buffer);

        log_info(">>>> %s", buffer);

        module = LoadLibraryA("ezusb-orig.dll");
        // module = LoadLibraryEx("ezusb-orig.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
        DWORD error = GetLastError();

        log_misc("loadlibrarya: ezusb-orig.dll: %p", module);

        if (!module) {
            if (error == ERROR_DLL_INIT_FAILED) {
                log_fatal("Initializing ezusb-orig.dll (DllMain) failed");
            } else {
                log_fatal("Could not load ezusb-orig.dll (%08lX).", error);
            }
        }

        real_usbCheckAlive = (usbCheckAlive_t) get_proc_address(module, "usbCheckAlive");
        real_usbCheckSecurityNew = (usbCheckSecurityNew_t) get_proc_address(module, "usbCheckSecurityNew");
        real_usbCoinGet = (usbCoinGet_t) get_proc_address(module, "usbCoinGet");
        real_usbCoinMode = (usbCoinMode_t) get_proc_address(module, "usbCoinMode");
        real_usbEnd = (usbEnd_t) get_proc_address(module, "usbEnd");
        real_usbFirmResult = (usbFirmResult_t) get_proc_address(module, "usbFirmResult");
        real_usbGetKEYID = (usbGetKEYID_t) get_proc_address(module, "usbGetKEYID");
        real_usbGetSecurity = (usbGetSecurity_t) get_proc_address(module, "usbGetSecurity");
        real_usbLamp = (usbLamp_t) get_proc_address(module, "usbLamp");
        real_usbPadRead = (usbPadRead_t) get_proc_address(module, "usbPadRead");
        real_usbPadReadLast = (usbPadReadLast_t) get_proc_address(module, "usbPadReadLast");
        real_usbSecurityInit = (usbSecurityInit_t) get_proc_address(module, "usbSecurityInit");
        real_usbSecurityInitDone = (usbSecurityInitDone_t) get_proc_address(module, "usbSecurityInitDone");
        real_usbSecuritySearch = (usbSecuritySearch_t) get_proc_address(module, "usbSecuritySearch");
        real_usbSecuritySearchDone = (usbSecuritySearchDone_t) get_proc_address(module, "usbSecuritySearchDone");
        real_usbSecuritySelect = (usbSecuritySelect_t) get_proc_address(module, "usbSecuritySelect");
        real_usbSecuritySelectDone = (usbSecuritySelectDone_t) get_proc_address(module, "usbSecuritySelectDone");
        real_usbSetExtIo = (usbSetExtIo_t) get_proc_address(module, "usbSetExtIo");
        real_usbStart = (usbStart_t) get_proc_address(module, "usbStart");
        real_usbWdtReset = (usbWdtReset_t) get_proc_address(module, "usbWdtReset");
        real_usbWdtStart = (usbWdtStart_t) get_proc_address(module, "usbWdtStart");
        real_usbWdtStartDone = (usbWdtStartDone_t) get_proc_address(module, "usbWdtStartDone");

        dll_main = get_dll_main_address(module);

        log_info("Calling ezusb-orig DllMain....: %p", dll_main);

        BOOL res = dll_main(module, DLL_PROCESS_ATTACH, 0);

        log_misc("Result ezusb-orig DllMain: %d", res);
    }
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        log_file = fopen("ezusb-proxy.log", "w+");
        log_to_writer(log_writer_file, log_file);

        log_info("DllMain process attach");

        fflush(log_file);
    } else if (reason == DLL_PROCESS_DETACH) {
        log_misc("DllMain process detach");

        fflush(log_file);
        fclose(log_file);
    }

    return TRUE;
}
