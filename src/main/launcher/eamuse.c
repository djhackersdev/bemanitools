#define LOG_MODULE "eamuse"

#include "avs-ext/property-node.h"

#include "iface-core/log.h"

#include "imports/avs-ea3.h"

void eamuse_init(const core_property_node_t *node)
{
    struct property_node *node_avs;

    log_assert(node);

    log_info("init");

    node_avs = avs_ext_property_node_avs_property_node_get(node);

    ea3_boot(node_avs);

    log_misc("init done");
}

void eamuse_fini()
{
    log_info("fini");

    ea3_shutdown();

    log_misc("fini done");
}