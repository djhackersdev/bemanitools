#define LOG_MODULE "ezusb-proxy"

#include <windows.h>

#include <dbghelp.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ezusb-proxy/ezusb.h"

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

    // usbCheckAlive seems to be the first call to the ezusb API
    //init_real_ezusb();

    // return real_usbCheckAlive();
    return 1;
}

int usbCheckSecurityNew()
{
    log_misc("usbCheckSecurityNew\n");

    // return real_usbCheckSecurityNew();
    return 0;
}

int usbCoinGet()
{
    log_misc("usbCoinGet\n");

    // return real_usbCoinGet();
    return 0;
}

int usbCoinMode()
{
    log_misc("usbCoinMode\n");

    // return real_usbCoinMode();
    return 0;
}

int usbEnd()
{
    log_misc("usbEnd\n");

    // return real_usbEnd();
    return 0;
}

int usbFirmResult()
{
    log_misc("usbFirmResult\n");

    // return real_usbFirmResult();
    return 0;
}

int usbGetKEYID()
{
    log_misc("usbGetKEYID\n");

    // return real_usbGetKEYID();
    return 0;
}

int usbGetSecurity()
{
    log_misc("usbGetSecurity\n");

    // return real_usbGetSecurity();
    return 0;
}

int usbLamp(uint32_t lamp_bits)
{
    log_misc("usbLamp\n");

    // return real_usbLamp(lamp_bits);
    return 0;
}

int usbPadRead(unsigned int *pad_bits)
{
    log_misc("usbPadRead\n");

    *pad_bits = 0;

    // return real_usbPadRead(pad_bits);
    return 0;
}

int usbPadReadLast(uint8_t *a1)
{
    log_misc("usbPadReadLast\n");

    memset(a1, 0, 40);

    // return real_usbPadReadLast(a1);
    return 0;
}

int usbSecurityInit()
{
    log_misc("usbSecurityInit\n");

    // return real_usbSecurityInit();
    return 0;
}

int usbSecurityInitDone()
{
    log_misc("usbSecurityInitDone\n");

    // return real_usbSecurityInitDone();
    return 0;
}

int usbSecuritySearch()
{
    log_misc("usbSecuritySearch\n");

    // return real_usbSecuritySearch();
    return 0;
}

int usbSecuritySearchDone()
{
    log_misc("usbSecuritySearchDone\n");

    // return real_usbSecuritySearchDone();
    return 0;
}

int usbSecuritySelect()
{
    log_misc("usbSecuritySelect\n");

    // return real_usbSecuritySelect();
    return 0;
}

int usbSecuritySelectDone()
{
    log_misc("usbSecuritySelectDone\n");

    // return real_usbSecuritySelectDone();
    return 0;
}

int usbSetExtIo()
{
    log_misc("usbSetExtIo\n");

    // return real_usbSetExtIo();
    return 0;
}

int usbStart()
{
    log_misc("usbStart\n");

    // return real_usbStart();
    return 0;
}

int usbWdtReset()
{
    log_misc("usbWdtReset\n");

    // return real_usbWdtReset();
    return 0;
}

int usbWdtStart(int a1)
{
    log_misc("usbWdtStart\n");

    // return real_usbWdtStart(a1);
    return 0;
}

int usbWdtStartDone()
{
    log_misc("usbWdtStartDone\n");

    // return real_usbWdtStartDone();
    return 0;
}

static void* get_proc_address(HMODULE module, const char* name)
{
    void* addr;

    log_misc("Resolving %s...", name);

    addr = GetProcAddress(module, name);

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

static void bootstrap_real_ezusb()
{
    HMODULE module;
    DllEntryPoint_t dll_main;

    log_info("Bootstrapping real ezusb library");

    module = LoadLibraryEx("ezusb-orig.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);

    log_misc("ezusb-orig.dll: %p", module);

    if (!module) {
        log_fatal("Could not load ezusb-orig.dll (%08lX).", GetLastError());
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

static void init_real_ezusb()
{
    if (!real_ezusb_initialized) {
        real_ezusb_initialized = true;

        bootstrap_real_ezusb();
    }
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        log_file = fopen("ezusb-proxy.log", "w+");
        log_to_writer(log_writer_file, log_file);

        log_info("DllMain process attach");
    } else if (reason == DLL_PROCESS_DETACH) {
        log_misc("DllMain process detach");

        fflush(log_file);
        fclose(log_file);
    }

    return TRUE;
}
