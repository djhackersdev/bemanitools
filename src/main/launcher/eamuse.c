#define LOG_MODULE "eamuse"

#include "core/log.h"

#include "imports/avs-ea3.h"

void eamuse_init(struct property_node *node)
{
    log_assert(node);

    log_info("init");

    ea3_boot(node);

    log_misc("init done");
}

void eamuse_fini()
{
    log_info("fini");

    ea3_shutdown();

    log_misc("fini done");
}