#define LOG_MODULE "avs-boot"

#include <stdint.h>
#include <string.h>

#include "hook/table.h"

#include "imports/avs.h"

#include "iidxhook3/avs-boot.h"

#include "util/log.h"

static void (*real_avs_boot)(
    struct property_node *config,
    void *std_heap,
    size_t sz_std_heap,
    void *avs_heap,
    size_t sz_avs_heap,
    avs_log_writer_t log_writer,
    void *log_context);
static int (*real_ea3_boot_avs)(struct property_node *config);
static int (*real_ea3_boot)(struct property_node *config);

static void my_avs_boot(
    struct property_node *config,
    void *std_heap,
    size_t sz_std_heap,
    void *avs_heap,
    size_t sz_avs_heap,
    avs_log_writer_t log_writer,
    void *log_context);
static int my_ea3_boot_avs(struct property_node *config);
static int my_ea3_boot(struct property_node *config);

static struct net_addr iidxhook3_avs_boot_eamuse_server_addr;

static const struct hook_symbol iidxhook3_avs_hook_syms[] = {
    {.name = "avs_boot",
     .patch = my_avs_boot,
     .link = (void **) &real_avs_boot},
    {.name = "ea3_boot",
     .patch = my_ea3_boot_avs,
     .link = (void **) &real_ea3_boot_avs},
};

static const struct hook_symbol iidxhook3_ea3_hook_syms[] = {
    {.name = "ea3_boot",
     .patch = my_ea3_boot,
     .link = (void **) &real_ea3_boot},
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
    } else {
        log_fatal("Could not avs_boot_replace_property_str(%s, %s)", name, val);
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

    real_avs_boot(
        config,
        std_heap,
        sz_std_heap,
        avs_heap,
        sz_avs_heap,
        log_writer_debug,
        NULL);
}

static void insert_eamuse_addr(struct property_node *config)
{
    char *server_addr;

    if (iidxhook3_avs_boot_eamuse_server_addr.type != NET_ADDR_TYPE_INVALID) {
        log_misc("Injecting network server address");

        server_addr = net_addr_to_str(&iidxhook3_avs_boot_eamuse_server_addr);

        // Remove protocol to avoid errors during ea3_boot.
        if (!strncmp(server_addr, "http://", strlen("http://"))) {
            server_addr += strlen("http://");
        } else if (!strncmp(server_addr, "https://", strlen("https://"))) {
            server_addr += strlen("https://");
        }

        avs_boot_replace_property_str(config, "network/services", server_addr);

        free(server_addr);
    }
}

static int my_ea3_boot_avs(struct property_node *config)
{
    log_info("Called my_ea3_boot_avs");
    insert_eamuse_addr(config);
    return real_ea3_boot_avs(config);
}

static int my_ea3_boot(struct property_node *config)
{
    log_info("Called my_ea3_boot");
    insert_eamuse_addr(config);
    return real_ea3_boot(config);
}

void iidxhook3_avs_boot_init()
{
    // IIDX 14 and 15 have the ea3_boot in libavs-win32.dll.
    hook_table_apply(
        NULL,
        "libavs-win32.dll",
        iidxhook3_avs_hook_syms,
        lengthof(iidxhook3_avs_hook_syms));

    hook_table_apply(
        NULL,
        "libavs-win32-ea3.dll",
        iidxhook3_ea3_hook_syms,
        lengthof(iidxhook3_ea3_hook_syms));

    memset(&iidxhook3_avs_boot_eamuse_server_addr, 0, sizeof(struct net_addr));

    log_info("Inserted avs log hooks");
}

void iidxhook3_avs_boot_set_eamuse_addr(const struct net_addr *server_addr)
{
    char *str;

    str = net_addr_to_str(server_addr);
    log_info("Setting eamuse server: %s", str);
    free(str);

    memcpy(
        &iidxhook3_avs_boot_eamuse_server_addr,
        server_addr,
        sizeof(struct net_addr));
}
