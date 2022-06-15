#define LOG_MODULE "avs-boot"

#include <stdint.h>
#include <string.h>

#include "hook/table.h"

#include "imports/avs.h"

#include "iidxhook4-cn/avs-boot.h"

#include "util/log.h"

static void (*real_avs_boot)(
    struct property_node *config,
    void *std_heap,
    size_t sz_std_heap,
    void *avs_heap,
    size_t sz_avs_heap,
    avs_log_writer_t log_writer,
    void *log_context);

static void my_avs_boot(
    struct property_node *config,
    void *std_heap,
    size_t sz_std_heap,
    void *avs_heap,
    size_t sz_avs_heap,
    avs_log_writer_t log_writer,
    void *log_context);

static const struct hook_symbol iidxhook4_cn_log_hook_syms[] = {
    {.name = "avs_boot",
     .patch = my_avs_boot,
     .link = (void **) &real_avs_boot},
};

static void avs_boot_replace_property_uint32(
    struct property_node *node, const char *name, uint32_t val)
{
    struct property_node *tmp;

    tmp = property_search(NULL, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    property_node_create(NULL, node, PSMAP_TYPE_U32, name, val);
}

static void avs_boot_replace_property_str(
    struct property_node *node, const char *name, const char *val)
{
    struct property_node *tmp;

    tmp = property_search(NULL, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    tmp = property_node_create(NULL, node, PROPERTY_TYPE_STR, name, val);

    if (tmp) {
        property_node_datasize(tmp);
    }
}

static void my_avs_boot(
    struct property_node *config,
    void *std_heap,
    size_t sz_std_heap,
    void *avs_heap,
    size_t sz_avs_heap,
    avs_log_writer_t log_writer,
    void *log_context)
{
    log_info("Called my_avs_boot");

    avs_boot_replace_property_uint32(config, "log/level", 4);
    avs_boot_replace_property_str(config, "fs/root/device", ".");

    real_avs_boot(
        config,
        std_heap,
        sz_std_heap,
        avs_heap,
        sz_avs_heap,
        log_writer_debug,
        NULL);
}

void iidxhook4_cn_avs_boot_init()
{
    hook_table_apply(
        NULL,
        "libavs-win32.dll",
        iidxhook4_cn_log_hook_syms,
        lengthof(iidxhook4_cn_log_hook_syms));

    log_info("Inserted avs log hooks");
}
