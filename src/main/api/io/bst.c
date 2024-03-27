
#include "api/lib.h"

#include "btapi/io/bst.h"

struct api_io_bst {
    api_lib_t lib;
    bt_io_bst_init_t init;
    bt_io_bst_fini_t fini;
    bt_io_bst_input_read_t input_read;
    bt_io_bst_input_get_t input_get;
};

static void _api_io_bst_resolve(api_io_bst_t *io)
{
    io->init = (bt_io_bst_init_t) api_lib_func_resolve(input->lib, "bt_io_bst_init", 1);
    io->fini = (bt_io_bst_fini_t) api_lib_func_resolve(input->lib, "bt_io_bst_fini", 1);
    io->input_read = (bt_io_bst_input_read_t) api_lib_func_resolve(input->lib, "bt_io_bst_input_read", 1);
    io->input_get = (bt_io_bst_input_get_t) api_lib_func_resolve(input->lib, "bt_io_bst_input_get", 1);
}

void api_io_bst_load(const char *path, api_io_bst_t **io)
{
    log_assert(path);
    log_assert(io);

    *io = xmalloc(sizeof(api_io_bst_t));
    memset(*io, 0, sizeof(api_io_bst_t));

    api_lib_load(path, (*io)->lib);
    _api_io_bst_resolve(io);
}

void api_io_bst_free(api_io_bst_t **io)
{
    log_assert(io);

    api_lib_free((*io)->lib);
    memset(*io, 0, sizeof(api_io_bst_t));
}

bool api_io_bst_init(const api_io_bst_t *io)
{
    bool result;

    log_assert(io);

    api_lib_func_pre_invoke_log(io->lib, "init");

    result = io->init();

    api_lib_func_post_invoke_log(io->lib, "init");

    return result;
}

void api_io_bst_fini(const api_io_bst_t *io)
{
    log_assert(io);

    api_lib_func_pre_invoke_log(io->lib, "fini");

    io->fini();

    api_lib_func_post_invoke_log(io->lib, "fini");
}

bool api_io_bst_input_read(const api_io_bst_t *io)
{
    log_assert(io);

    // Do not log on frequently invoked calls to avoid negative performance impact and log spam

    return io->input_read();
}

uint8_t api_io_bst_input_get(const api_io_bst_t *io)
{
    log_assert(io);

    // Do not log on frequently invoked calls to avoid negative performance impact and log spam

    return io->input_get();
}
