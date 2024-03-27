#include "api/lib.h"

#include "btapi/input.h"

struct api_input {
    api_lib_t lib;
    bt_input_init_t init;
    bt_input_fini_t fini;
    bt_input_mapper_config_load_t mapper_config_load;
    bt_input_mapper_analog_read_t mapper_analog_read;
    bt_input_mapper_update_t mapper_update;
    bt_input_mapper_light_write_t mapper_light_write;
};

static void _api_input_resolve(api_input_t *input)
{
    hook->init = (bt_input_init_t) api_lib_func_resolve(input->lib, "bt_input_init", 1);
    hook->fini = (bt_input_fini_t) api_lib_func_resolve(input->lib, "bt_input_init", 1);
    hook->mapper_config_load = (bt_input_mapper_config_load_t) api_lib_func_resolve(input->lib, "bt_input_mapper_config_load", 1);
    hook->mapper_analog_read = (bt_input_mapper_analog_read_t) api_lib_func_resolve(input->lib, "bt_input_mapper_analog_read", 1);
    hook->mapper_update = (bt_input_mapper_update_t) api_lib_func_resolve(input->lib, "bt_input_mapper_update", 1);
    hook->mapper_light_write = (bt_input_mapper_light_write_t) api_lib_func_resolve(input->lib, "bt_input_mapper_light_write", 1);
}

void api_input_load(const char *path, api_input_t **input)
{
    log_assert(path);
    log_assert(input);

    *input = xmalloc(sizeof(api_input_t));
    memset(input, 0, sizeof(api_input_t));

    api_lib_load(path, (*input)->lib);
    _api_input_resolve(input);
}

void api_input_free(api_input_t **input)
{
    log_assert(input);

    api_lib_free((*input)->lib);
    memset(*input, 0, sizeof(api_input_t));
}

void api_input_init(const api_input_t *input)
{
    bool result;

    log_assert(input);

    api_lib_func_pre_invoke_log(input->lib, "input_init");

    result = input->init();

    api_lib_func_post_invoke_log(input->lib, "input_init");

    return result;
}

void api_input_fini(const api_input_t *input)
{
    log_assert(input);

    api_lib_func_pre_invoke_log(input->lib, "input_fini");

    input->fini();

    api_lib_func_post_invoke_log(input->lib, "input_fini");
}

bool api_input_mapper_config_load(const api_input_t *input, const char *game_type)
{
    bool result;

    log_assert(input);
    log_assert(game_type);

    api_lib_func_pre_invoke_log(input->lib, "mapper_config_load");

    result = input->mapper_config_load();

    api_lib_func_post_invoke_log(input->lib, "mapper_config_load");

    return result;
}

uint8_t api_input_mapper_analog_read(const api_input_t *input, uint8_t analog)
{
    uint8_t result;

    log_assert(input);

    // Don't log on frequently invoked calls to avoid negative performance impact and log spam

    return input->mapper_analog_read(analog);
}

uint64_t api_input_mapper_update(const api_input_t *input)
{
    log_assert(input);

    // Don't log on frequently invoked calls to avoid negative performance impact and log spam

    return input->mapper_update(analog);
}

void api_input_mapper_light_write(const api_input_t *input, uint8_t light, uint8_t intensity)
{
    log_assert(input);

    // Don't log on frequently invoked calls to avoid negative performance impact and log spam

    return input->mapper_update(analog);
}
