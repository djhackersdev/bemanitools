// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <commctrl.h>
// clang-format on

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config/gametype.h"
#include "config/resource.h"
#include "config/schema.h"
#include "config/spinner.h"
#include "config/usages.h"

#include "core/boot.h"
#include "core/log-bt.h"
#include "core/log-bt-ext.h"
#include "core/thread-crt.h"

#include "eamio/eam-config.h"

#include "geninput/input-config.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface-io/eam.h"
#include "iface/input.h"

#include "module/input-ext.h"
#include "module/input.h"
#include "module/io-ext.h"
#include "module/io.h"

#include "util/defs.h"
#include "util/str.h"
#include "util/winres.h"

HPROPSHEETPAGE
analogs_ui_tab_create(HINSTANCE inst, const struct schema *schema);
HPROPSHEETPAGE buttons_tab_create(HINSTANCE inst, const struct schema *schema);
HPROPSHEETPAGE lights_tab_create(HINSTANCE inst, const struct schema *schema);
HPROPSHEETPAGE
eam_ui_tab_create(
    HINSTANCE inst,
    const struct schema *schema,
    const bt_io_eam_config_api_t *eam_io_config_api);

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

static void log_api_set()
{
    bt_core_log_api_t api;

    core_log_bt_core_api_get(&api);

    api.v1.fatal = my_fatal;

    bt_core_log_api_set(&api);
}

int main(int argc, char **argv)
{
    INITCOMMONCONTROLSEX iccx;
    HINSTANCE inst;
    HPROPSHEETPAGE psp[4];
    PROPSHEETHEADER psh;
    intptr_t result;
    const bt_io_eam_config_api_t *eam_io_config_api;
    const struct schema *schema;
    wchar_t text[1024];
    int max_light;
    size_t i;
    bt_input_api_t input_api;
    module_input_t *module_input;
    bt_io_eam_api_t eam_api;
    module_io_t *module_io_eam;

    // TODO turn this into optional parameters, same for log level and output to
    // file
    AttachConsole(ATTACH_PARENT_PROCESS);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);

    core_boot("config");

    core_log_bt_ext_init_with_stderr();

    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);
    core_log_bt_core_api_set();

    core_thread_crt_core_api_set();

    log_api_set();

    inst = GetModuleHandle(NULL);

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

    module_input_ext_load_and_init("geninput.dll", &module_input);
    module_input_api_get(module_input, &input_api);
    bt_input_api_set(&input_api);

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", &module_io_eam);
    module_io_api_get(module_io_eam, &eam_api);
    bt_io_eam_api_set(&eam_api);

    eam_io_config_api = bt_io_eam_config_api_get();

    // calculate these and check against the loaded config
    max_light = -1;

    for (i = 0; i < schema->nlights; i++) {
        if (max_light < schema->lights[i].bit) {
            max_light = schema->lights[i].bit;
        }
    }

    if (!bt_input_mapper_config_load(schema->name)) {
        log_info("Initializing empty config for %s", schema->name);

        log_info(">>>>1");
        mapper_set_nlights((uint8_t) (max_light + 1));
        log_info(">>>>1");
        mapper_set_nanalogs((uint8_t) schema->nanalogs);
        log_info(">>>>1");
    } else {
        // make sure that these are right

        if (mapper_get_nlights() != (max_light + 1)) {
            log_info("Updating nlights for %s", schema->name);
            mapper_set_nlights((uint8_t) (max_light + 1));
        }

        if (mapper_get_nanalogs() != schema->nanalogs) {
            log_info("Updating nanalogs for %s", schema->name);
            mapper_set_nanalogs((uint8_t) schema->nanalogs);
        }
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

    bt_io_eam_fini();
    bt_input_fini();
    spinner_fini(inst);
    usages_fini();

    bt_input_api_clear();
    bt_io_eam_api_clear();

    module_input_free(&module_input);
    module_io_free(&module_io_eam);

    return EXIT_SUCCESS;
}
