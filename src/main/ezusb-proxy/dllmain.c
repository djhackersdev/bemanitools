#define LOG_MODULE "ezusb-proxy"

#include <windows.h>
#include <setupapi.h>

#include <dbghelp.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/table.h"

#include "ezusb-proxy/ezusb-proxy.h"

#include "util/log.h"

static HDEVINFO STDCALL my_SetupDiGetClassDevsA(
    const GUID *class_guid, const char *enumerator, HWND hwnd, DWORD flags);

static HDEVINFO(STDCALL *real_SetupDiGetClassDevsA)(
    const GUID *class_guid, const char *enumerator, HWND hwnd, DWORD flags);

static const struct hook_symbol setupapi_hook_syms[] = {
    {.name = "SetupDiGetClassDevsA",
     .patch = my_SetupDiGetClassDevsA,
     .link = (void **) &real_SetupDiGetClassDevsA},
};

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

static HDEVINFO STDCALL my_SetupDiGetClassDevsA(
    const GUID *class_guid, const char *enumerator, HWND hwnd, DWORD flags)
{
    /* That's how iidx 9-13 detected the old C02 IO. That doesn't work
       on iidx 14 anymore... */
    /*
    if (class_guid != NULL
            && memcmp(class_guid, &hook_setupapi_data->device_guid,
            sizeof(hook_setupapi_data->device_guid)) == 0) {
    */

   if (flags == 0x573573) {
       log_info("!!!!!!!!!!!!");
       return 0;
   }

    log_misc("my_SetupDiGetClassDevsA");

    return real_SetupDiGetClassDevsA(class_guid, enumerator, hwnd, flags);
}

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

/*
        HANDLE cur_process = GetCurrentProcess();

        BOOL result;

        while (true) {
            result = FALSE;

            if (!CheckRemoteDebuggerPresent(cur_process, &result)) {
                log_fatal(
                    "ERROR: CheckRemoteDebuggerPresent failed: %08x",
                    (unsigned int) GetLastError());
            }

            if (result) {
                log_info("Remote debugger attached, resuming");
                break;
            }

            Sleep(1000);
        }
*/

        log_info("Loading real ezusb library");

        GetCurrentDirectoryA(MAX_PATH, buffer);

        log_info(">>>> %s", buffer);

        // module = LoadLibraryA("ezusb-orig.dll");

        // TODO according to the docs, this does not resolve dependencies, so loading of the
        // original ezusb might be incomplete
        module = LoadLibraryEx("ezusb-orig.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
        DWORD error = GetLastError();

        log_misc("loadlibrarya: ezusb-orig.dll: %p", module);

        if (!module) {
            if (error == ERROR_DLL_INIT_FAILED) {
                log_fatal("Initializing ezusb-orig.dll (DllMain) failed");
            } else {
                log_fatal("Could not load ezusb-orig.dll (%08lX).", error);
            }
        }
        
        log_info("Loading setupapi.dll");

        if (LoadLibraryA("setupapi.dll") == NULL) {
            log_fatal("Loading setupapi.dll failed");
        }

        hook_table_apply(
            NULL, "setupapi.dll", setupapi_hook_syms, lengthof(setupapi_hook_syms));

        log_info("test call setupdigetclassdevsa");
        SetupDiGetClassDevsA(0, 0, 0, 0x573573);



        // Use mangled symbol names because these are the full export names
        // Otherwise, lookups result in functions not being found
        real_usbCheckAlive = (usbCheckAlive_t) get_proc_address(module, "?usbCheckAlive@@YAHXZ");
        real_usbCheckSecurityNew = (usbCheckSecurityNew_t) get_proc_address(module, "?usbCheckSecurityNew@@YAHH@Z");
        real_usbCoinGet = (usbCoinGet_t) get_proc_address(module, "?usbCoinGet@@YAHH@Z");
        real_usbCoinMode = (usbCoinMode_t) get_proc_address(module, "?usbCoinMode@@YAHH@Z");
        real_usbEnd = (usbEnd_t) get_proc_address(module, "?usbEnd@@YAHXZ");
        real_usbFirmResult = (usbFirmResult_t) get_proc_address(module, "?usbFirmResult@@YAHXZ");
        real_usbGetKEYID = (usbGetKEYID_t) get_proc_address(module, "?usbGetKEYID@@YAHPAEH@Z");
        real_usbGetSecurity = (usbGetSecurity_t) get_proc_address(module, "?usbGetSecurity@@YAHHPAE@Z");
        real_usbLamp = (usbLamp_t) get_proc_address(module, "?usbLamp@@YAHH@Z");
        real_usbPadRead = (usbPadRead_t) get_proc_address(module, "?usbPadRead@@YAHPAK@Z");
        real_usbPadReadLast = (usbPadReadLast_t) get_proc_address(module, "?usbPadReadLast@@YAHPAE@Z");
        real_usbSecurityInit = (usbSecurityInit_t) get_proc_address(module, "?usbSecurityInit@@YAHXZ");
        real_usbSecurityInitDone = (usbSecurityInitDone_t) get_proc_address(module, "?usbSecurityInitDone@@YAHXZ");
        real_usbSecuritySearch = (usbSecuritySearch_t) get_proc_address(module, "?usbSecuritySearch@@YAHXZ");
        real_usbSecuritySearchDone = (usbSecuritySearchDone_t) get_proc_address(module, "?usbSecuritySearchDone@@YAHXZ");
        real_usbSecuritySelect = (usbSecuritySelect_t) get_proc_address(module, "?usbSecuritySelect@@YAHH@Z");
        real_usbSecuritySelectDone = (usbSecuritySelectDone_t) get_proc_address(module, "?usbSecuritySelectDone@@YAHXZ");
        real_usbSetExtIo = (usbSetExtIo_t) get_proc_address(module, "?usbSetExtIo@@YAHH@Z");
        real_usbStart = (usbStart_t) get_proc_address(module, "?usbStart@@YAHH@Z");
        real_usbWdtReset = (usbWdtReset_t) get_proc_address(module, "?usbWdtReset@@YAHXZ");
        real_usbWdtStart = (usbWdtStart_t) get_proc_address(module, "?usbWdtStart@@YAHH@Z");
        real_usbWdtStartDone = (usbWdtStartDone_t) get_proc_address(module, "?usbWdtStartDone@@YAHXZ");

        dll_main = get_dll_main_address(module);

        log_info("Waiting for debugger to be attached");

        HANDLE cur_process = GetCurrentProcess();

        BOOL result;

        while (true) {
            result = FALSE;

            if (!CheckRemoteDebuggerPresent(cur_process, &result)) {
                log_fatal(
                    "ERROR: CheckRemoteDebuggerPresent failed: %08x",
                    (unsigned int) GetLastError());
            }

            if (result) {
                log_info("Remote debugger attached, resuming");
                break;
            }

            Sleep(1000);
        }

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
