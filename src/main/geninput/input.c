#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include "bemanitools/glue.h"
#include "bemanitools/input.h"

#include "geninput/hid-mgr.h"
#include "geninput/hid.h"
#include "geninput/io-thread.h"
#include "geninput/mapper-s11n.h"
#include "geninput/mapper.h"

#include "util/fs.h"
#include "util/log.h"
#include "util/msg-thread.h"
#include "util/str.h"
#include "util/thread.h"

static HINSTANCE input_hinst;
static volatile long input_init_count;

static FILE *mapper_config_open(const char *game_type, const char *mode)
{
    char path[MAX_PATH];
    FILE *f;

    str_format(path, sizeof(path), "%s.bin", game_type);
    f = fopen_appdata("DJHACKERS", path, mode);

    // Try to load the old save file only if we're loading
    if (f == NULL) {
        if (strchr(mode, 'r') != NULL) {
            str_format(path, sizeof(path), "%s_v5_00a07.bin", game_type);
            f = fopen_appdata("DJHACKERS", path, mode);
        }
    }
    
    return f;
}

void input_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    log_to_external(misc, info, warning, fatal);
}

void input_init(
    thread_create_t create, thread_join_t join, thread_destroy_t destroy)
{
    if (InterlockedIncrement(&input_init_count) != 1) {
        return;
    }

    log_info("Generic input subsystem is starting up");

    hid_mgr_init();

    mapper_inst = mapper_impl_create();

    thread_api_init(create, join, destroy);
    msg_thread_init(input_hinst);
    io_thread_init();
}

void input_fini(void)
{
    if (InterlockedDecrement(&input_init_count) != 0) {
        return;
    }

    msg_thread_fini();
    io_thread_fini();

    mapper_impl_destroy(mapper_inst);
    hid_mgr_fini();

    log_info("Generic input subsystem has shut down");
}

bool mapper_config_load(const char *game_type)
{
    struct mapper *m;
    FILE *f;

    f = mapper_config_open(game_type, "rb");

    if (f == NULL) {
        goto open_fail;
    }

    m = mapper_impl_config_load(f);

    if (m == NULL) {
        goto read_fail;
    }

    mapper_impl_destroy(mapper_inst);
    mapper_inst = m;

    fclose(f);

    return true;

read_fail:
    fclose(f);

open_fail:
    return false;
}

void mapper_config_save(const char *game_type)
{
    FILE *f;

    f = mapper_config_open(game_type, "wb");

    if (f == NULL) {
        return;
    }

    mapper_impl_config_save(mapper_inst, f);
    fclose(f);
}

void mapper_clear_action_map(uint8_t action, uint8_t page)
{
    mapper_impl_clear_action_map(mapper_inst, action, page);
}

void mapper_clear_light_map(const struct mapped_light *ml)
{
    mapper_impl_clear_light_map(mapper_inst, ml);
}

bool mapper_get_action_map(
    uint8_t action, uint8_t page, struct mapped_action *ma)
{
    return mapper_impl_get_action_map(mapper_inst, action, page, ma);
}

bool mapper_get_analog_map(uint8_t analog, struct mapped_analog *ma)
{
    return mapper_impl_get_analog_map(mapper_inst, analog, ma);
}

int32_t mapper_get_analog_sensitivity(uint8_t analog)
{
    return mapper_impl_get_analog_sensitivity(mapper_inst, analog);
}

bool mapper_get_analog_invert(uint8_t analog)
{
    return mapper_impl_get_analog_invert(mapper_inst, analog);
}

uint8_t mapper_get_nanalogs(void)
{
    return mapper_impl_get_nanalogs(mapper_inst);
}

uint8_t mapper_get_nlights(void)
{
    return mapper_impl_get_nlights(mapper_inst);
}

uint8_t mapper_get_npages(void)
{
    return mapper_impl_get_npages(mapper_inst);
}

bool mapper_is_analog_absolute(uint8_t analog)
{
    return mapper_impl_is_analog_absolute(mapper_inst, analog);
}

action_iter_t mapper_iterate_actions(void)
{
    return mapper_impl_iterate_actions(mapper_inst);
}

light_iter_t mapper_iterate_lights(void)
{
    return mapper_impl_iterate_lights(mapper_inst);
}

void mapper_set_action_map(
    uint8_t action, uint8_t page, uint8_t bit, const struct mapped_action *ma)
{
    mapper_impl_set_action_map(mapper_inst, action, page, bit, ma);
}

bool mapper_set_analog_map(uint8_t analog, const struct mapped_analog *ma)
{
    return mapper_impl_set_analog_map(mapper_inst, analog, ma);
}

bool mapper_set_analog_sensitivity(uint8_t analog, int32_t sensitivity)
{
    return mapper_impl_set_analog_sensitivity(mapper_inst, analog, sensitivity);
}

bool mapper_set_analog_invert(uint8_t analog, bool invert)
{
    return mapper_impl_set_analog_invert(mapper_inst, analog, invert);
}

void mapper_set_light_map(const struct mapped_light *ml, uint8_t game_light)
{
    mapper_impl_set_light_map(mapper_inst, ml, game_light);
}

void mapper_set_nanalogs(uint8_t nanalogs)
{
    mapper_impl_set_nanalogs(mapper_inst, nanalogs);
}

void mapper_set_nlights(uint8_t nlights)
{
    mapper_impl_set_nlights(mapper_inst, nlights);
}

uint8_t mapper_read_analog(uint8_t analog)
{
    return mapper_impl_read_analog(mapper_inst, analog);
}

uint64_t mapper_update(void)
{
    return mapper_impl_update(mapper_inst);
}

void mapper_write_light(uint8_t light, uint8_t intensity)
{
    mapper_impl_write_light(mapper_inst, light, intensity);
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        input_hinst = hinst;
    }

    return TRUE;
}
