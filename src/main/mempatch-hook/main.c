#include <windows.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"

#include "iface-core/log.h"

#include "util/cmdline.h"
#include "util/fs.h"
#include "util/hex.h"

static bool patch_memory_check_data(
    uintptr_t base_address,
    uintptr_t address,
    uint8_t *data_expected,
    size_t len)
{
    uint8_t *dest = (uint8_t *) (base_address + address);
    bool success = true;

    for (size_t i = 0; i < len; i++) {
        if (dest[i] != data_expected[i]) {
            log_warning(
                "Memcheck error: base %lluX + address %lluX + offset %d: "
                "expected %X, found %X",
                (uint64_t) base_address,
                (uint64_t) address,
                // this cast is technically wrong
                // but no one should be patching 32K worth of bytes at once
                (int) i,
                data_expected[i],
                dest[i]);
            success = false;
        }
    }

    return success;
}

static bool patch_memory_data(
    uintptr_t base_address, uintptr_t address, uint8_t *data, size_t len)
{
    DWORD old_protect;

    uint8_t *dest = (uint8_t *) (base_address + address);

    if (!VirtualProtect(
            (void *) dest,
            sizeof(uint8_t) * len,
            PAGE_EXECUTE_READWRITE,
            &old_protect)) {
        log_warning(
            "VirtualProtect %p (%Iu) failed: %d",
            dest,
            len,
            (int) GetLastError());
        return false;
    }

    memcpy((void *) (base_address + address), data, len);

    if (!VirtualProtect(
            (void *) dest, sizeof(uint8_t) * len, old_protect, &old_protect)) {
        log_warning(
            "VirtualProtect (2) %p (%Iu) failed: %d",
            dest,
            len,
            (int) GetLastError());
        return false;
    }

    return true;
}

static bool patch_memory_apply(char *script)
{
    char *pos_lines;
    char *ctx_lines;
    uint32_t line_cnt;

    char *pos_line;
    char *ctx_line;
    uint32_t line_tokens;
    bool use_crlf;

    char *module;
    char *address_str = NULL;
    char *data_str = NULL;
    size_t data_str_len;
    char *data_expected_str = NULL;
    size_t data_expected_str_len;

    uintptr_t address;
    uintptr_t address_base;
    HMODULE hmodule;
    uint8_t data[4096];
    size_t data_len;
    uint8_t data_expected[4096];
    size_t data_expected_len;

    bool error;

    error = false;

    line_cnt = 0;

    use_crlf = false;
    if (strstr(script, "\r\n")) {
        use_crlf = true;
    }
    pos_lines = strtok_r(script, use_crlf ? "\r\n" : "\n", &ctx_lines);

    while (pos_lines != NULL) {
        /* ignore comments and empty lines */
        if (strlen(pos_lines) > 0 && pos_lines[0] != '#') {
            log_misc("[%d] Parsing: %s", line_cnt, pos_lines);

            pos_line = strtok_r(pos_lines, " ", &ctx_line);
            line_tokens = 0;

            while (pos_line != NULL) {
                switch (line_tokens) {
                    case 0:
                        module = pos_line;
                        break;

                    case 1:
                        address_str = pos_line;
                        break;

                    case 2:
                        data_str = pos_line;
                        break;

                    case 3:
                        data_expected_str = pos_line;
                        break;

                    default:
                        break;
                }

                pos_line = strtok_r(NULL, " ", &ctx_line);
                line_tokens++;
            }

            if (line_tokens != 3 && line_tokens != 4) {
                log_warning(
                    "[%d] Invalid number of elements in line '%s' skipping",
                    line_cnt,
                    pos_lines);
                goto error_next_line;
            }

            if (!strcmp(module, "-")) {
                address_base = 0;
            } else {
                hmodule = GetModuleHandleA(module);

                if (hmodule == NULL) {
                    log_warning(
                        "[%d] Could not find module %s", line_cnt, module);
                    goto error_next_line;
                }

                address_base = (uintptr_t) hmodule;
            }

            address = strtoll(address_str, NULL, 16);

            /* Most likely an error but we don't know for sure */
            if (address == 0) {
                log_warning("[%d] Address specified is 0", line_cnt);
            }

            data_str_len = strlen(data_str);
            data_len = data_str_len;

            if (data_len % 2 != 0) {
                log_warning(
                    "[%d] Data length %Iu mod 2 != 0: %s",
                    line_cnt,
                    data_len,
                    data_str);
                goto error_next_line;
            }

            data_len /= 2;

            if (line_tokens == 4) {
                data_expected_str_len = strlen(data_expected_str);
                data_expected_len = data_expected_str_len;

                if (data_expected_len % 2 != 0) {
                    log_warning(
                        "[%d] Data expected length %Iu mod 2 != 0: %s",
                        line_cnt,
                        data_len,
                        data_str);
                    goto error_next_line;
                }
            } else {
                data_expected_len = 0;
            }

            data_expected_len /= 2;

            if (!hex_decode(data, sizeof(data), data_str, data_str_len)) {
                log_warning(
                    "[%d] Decoding data (item 3) failed: %s",
                    line_cnt,
                    data_str);
                goto error_next_line;
            }

            if (line_tokens == 4) {
                if (!hex_decode(
                        data_expected,
                        sizeof(data_expected),
                        data_expected_str,
                        data_expected_str_len)) {
                    log_warning(
                        "[%d] Decoding expected data (item 4) failed: %s",
                        line_cnt,
                        data_expected_str);
                    goto error_next_line;
                }
            }

            if (data_expected_len) {
                if (!patch_memory_check_data(
                        address_base,
                        address,
                        data_expected,
                        data_expected_len)) {
                    log_warning("[%d] Memcheck failed, skipping", line_cnt);
                    goto error_next_line;
                }
            }

            if (!patch_memory_data(address_base, address, data, data_len)) {
                log_warning("[%d] Patching failed, skipping", line_cnt);
                goto error_next_line;
            }

            log_info("[%d] Patch ok", line_cnt);
        }

        goto next_line;

    error_next_line:
        error = true;

    next_line:
        pos_lines = strtok_r(NULL, use_crlf ? "\r\n" : "\n", &ctx_lines);
        line_cnt++;
    }

    return !error;
}

static void patch_memory_from_file(const char *filepath)
{
    char *buffer;
    size_t len;

    if (!file_load(filepath, (void **) &buffer, &len, true)) {
        log_fatal("Load script file %s failed", filepath);
    } else {
        if (!patch_memory_apply(buffer)) {
            log_fatal(
                "Applying one or multiple patches failed, see log output "
                "for details");
        }
    }

    free(buffer);
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        int argc;
        char **argv;
        char *filepath;
        bool patched;

        patched = false;

        core_log_bt_core_api_set();

        core_log_bt_ext_init_with_stderr_and_file(
            "mempatch.log", false, false, 0);
        core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);

        args_recover(&argc, &argv);

        for (int i = 0; i < argc; i++) {
            /* allow specifying multiple mempatch files */
            if (!strcmp(argv[i], "--mempatch") && i + 1 < argc) {
                filepath = argv[i + 1];
                log_info("Loading memory patch file: %s", filepath);

                patch_memory_from_file(filepath);

                log_info("Finished patching with file %s", filepath);
                i++;
                patched = true;
            }
        }

        if (!patched) {
            log_info(
                "No files specified for patching. Add the parameter "
                "--mempatch <filepath> to your application arguments. This "
                "can "
                "be specified multiple times, e.g. --mempatch file1.mph "
                "--mempatch file2.mph. The patches are executed in the "
                "order "
                "they are specified.");
        } else {
            log_info("Patching done");
        }

        core_log_bt_fini();
    }

    return TRUE;
}