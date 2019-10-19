#include <commctrl.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config/gametype.h"
#include "config/resource.h"
#include "config/schema.h"
#include "config/spinner.h"
#include "config/usages.h"

#include "eamio/eam-config.h"

#include "geninput/input-config.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/thread.h"
#include "util/winres.h"

HPROPSHEETPAGE
analogs_ui_tab_create(HINSTANCE inst, const struct schema *schema);
HPROPSHEETPAGE buttons_tab_create(HINSTANCE inst, const struct schema *schema);
HPROPSHEETPAGE lights_tab_create(HINSTANCE inst, const struct schema *schema);
HPROPSHEETPAGE
eam_ui_tab_create(
    HINSTANCE inst,
    const struct schema *schema,
    const struct eam_io_config_api *eam_io_config_api);

static void my_fatal(const char *module, const char *fmt, ...)
{
    va_list ap;
    char buf[1024];

    va_start(ap, fmt);
    str_vformat(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    MessageBoxA(NULL, buf, NULL, MB_ICONERROR | MB_OK);
    DebugBreak();

    ExitProcess(-1);
}

int main(int argc, char **argv)
{
    INITCOMMONCONTROLSEX iccx;
    HINSTANCE inst;
    HPROPSHEETPAGE psp[4];
    PROPSHEETHEADER psh;
    intptr_t result;
    const struct eam_io_config_api *eam_io_config_api;
    const struct schema *schema;
    wchar_t text[1024];
    int max_light;
    size_t i;

    inst = GetModuleHandle(NULL);

    log_to_writer(log_writer_debug, NULL);
    log_to_external(log_impl_misc, log_impl_info, log_impl_warning, my_fatal);

    usages_init(inst);

    memset(&iccx, 0, sizeof(iccx));
    iccx.dwSize = sizeof(iccx);
    iccx.dwICC = ICC_STANDARD_CLASSES | ICC_TAB_CLASSES | ICC_WIN95_CLASSES;

    if (!InitCommonControlsEx(&iccx)) {
        log_fatal("InitCommonControlsEx failed");
    }

    spinner_init(inst);

    if (argc == 2) {
        schema = game_type_from_str(argv[1]);

        if (schema == NULL) {
            rswprintf(text, lengthof(text), inst, IDS_BAD_GAME_TYPE, argv[1]);
            MessageBox(NULL, text, NULL, MB_ICONERROR | MB_OK);

            return EXIT_FAILURE;
        }
    } else {
        schema = game_type_from_dialog(inst);

        if (schema == NULL) {
            return EXIT_FAILURE;
        }
    }

    input_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);
    input_init(crt_thread_create, crt_thread_join, crt_thread_destroy);

    eam_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);
    eam_io_init(crt_thread_create, crt_thread_join, crt_thread_destroy);
    eam_io_config_api = eam_io_get_config_api();

    if (!mapper_config_load(schema->name)) {
        log_info("Initializing empty config for %s", schema->name);

        max_light = -1;

        for (i = 0; i < schema->nlights; i++) {
            if (max_light < schema->lights[i].bit) {
                max_light = schema->lights[i].bit;
            }
        }

        mapper_set_nlights((uint8_t)(max_light + 1));
        mapper_set_nanalogs((uint8_t) schema->nanalogs);
    }

    memset(&psh, 0, sizeof(psh));
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP | PSH_USEHICON;
    psh.hInstance = inst;
    psh.hIcon = LoadIcon(NULL, IDI_QUESTION);
    psh.pszCaption = MAKEINTRESOURCE(IDS_APPNAME);
    psh.phpage = psp;
    psh.nPages = 0;

    psp[psh.nPages++] = buttons_tab_create(inst, schema);
    psp[psh.nPages++] = lights_tab_create(inst, schema);

    if (schema->nanalogs > 0) {
        psp[psh.nPages++] = analogs_ui_tab_create(inst, schema);
    }

    if (eam_io_config_api != NULL) {
        psp[psh.nPages++] = eam_ui_tab_create(inst, schema, eam_io_config_api);
    }

    /* Run GUI */

    result = PropertySheet(&psh);

    /* Save settings and clean up */

    if (result > 0) {
        if (eam_io_config_api != NULL) {
            eam_io_config_api->config_save();
        }

        mapper_config_save(schema->name);
    }

    eam_io_fini();
    input_fini();
    spinner_fini(inst);
    usages_fini();

    return EXIT_SUCCESS;
}
