#define LOG_MODULE "avs-boot"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hook/table.h"

#include "imports/avs.h"

#include "jbhook1/avs-boot.h"

#include "util/log.h"

static void (*real_avs_boot)(
        struct property_node *config, void *std_heap, size_t sz_std_heap,
        void *avs_heap, size_t sz_avs_heap, avs_log_writer_t log_writer,
        void *log_context);
static int (*real_log_change_level)(int level);
static int (*real_ea3_boot)(struct property_node *config);

static void my_avs_boot(
        struct property_node *config, void *std_heap, size_t sz_std_heap,
        void *avs_heap, size_t sz_avs_heap, avs_log_writer_t log_writer,
        void *log_context);
static int my_log_change_level(int level);
static int my_ea3_boot(struct property_node *config);

static struct net_addr jbhook1_avs_boot_eamuse_server_addr;

static const struct hook_symbol jbhook1_log_gftools_hook_syms[] = {
    {
        .name       = "avs_boot",
        .patch      = my_avs_boot,
        .link       = (void **) &real_avs_boot
    },
    {
        .name       = "log_change_level",
        .patch      = my_log_change_level,
        .link       = (void **) &real_log_change_level
    },
};

static const struct hook_symbol jbhook1_log_gftools_hook_syms2[] = {
    {
        .name       = "ea3_boot",
        .patch      = my_ea3_boot,
        .link       = (void **) &real_ea3_boot
    },
};

static void avs_boot_replace_property_uint32(struct property_node* node,
        const char* name, uint32_t val)
{
    struct property_node* tmp;
    
    tmp = property_search(NULL, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    property_node_create(NULL, node, PSMAP_TYPE_U32, name, val);
}

static void avs_boot_replace_property_str(struct property_node* node,
        const char* name, const char* val)
{
    struct property_node* tmp;
    
    tmp = property_search(NULL, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    tmp = property_node_create(NULL, node, PSMAP_TYPE_STR_LEGACY, name, val);

    if (tmp) {
        property_node_datasize(tmp);
    }
}

static void my_avs_boot(
        struct property_node *config, void *std_heap, size_t sz_std_heap,
        void *avs_heap, size_t sz_avs_heap, avs_log_writer_t log_writer,
        void *log_context)
{
    log_info("Called my_avs_boot");

    avs_boot_replace_property_uint32(config, "log/level", 4);
    avs_boot_replace_property_str(config, "/config/fs/nvram/device",
        "./CONF/NVRAM");
    avs_boot_replace_property_str(config, "/config/fs/raw/device",
        "./CONF/RAW");
    
    real_avs_boot(config, std_heap, sz_std_heap, avs_heap, sz_avs_heap, 
        log_writer_debug, NULL);
}

static int my_ea3_boot(struct property_node *config)
{
    char* server_addr;

    log_info("Called my_ea3_boot");

    if (jbhook1_avs_boot_eamuse_server_addr.type != NET_ADDR_TYPE_INVALID) {
        log_misc("Injecting network server address");

        server_addr = net_addr_to_str(&jbhook1_avs_boot_eamuse_server_addr);

        avs_boot_replace_property_str(config, "network/services", server_addr);

        free(server_addr);
    }

    return real_ea3_boot(config);
}

static int my_log_change_level(int level)
{
    log_misc("Log change level: %d", level);

    return real_log_change_level(level);
}

void jbhook1_avs_boot_init()
{
    hook_table_apply(
            NULL,
            "libavs-win32.dll",
            jbhook1_log_gftools_hook_syms,
            lengthof(jbhook1_log_gftools_hook_syms));

    hook_table_apply(
            NULL,
            "libavs-win32-ea3.dll",
            jbhook1_log_gftools_hook_syms2,
            lengthof(jbhook1_log_gftools_hook_syms2));

    memset(&jbhook1_avs_boot_eamuse_server_addr, 0, sizeof(struct net_addr));

    log_info("Inserted avs log hooks");
}

void jbhook1_avs_boot_set_eamuse_addr(const struct net_addr* server_addr)
{
    char* str;

    str = net_addr_to_str(server_addr);
    log_info("Setting eamuse server: %s", str);
    free(str);

    memcpy(&jbhook1_avs_boot_eamuse_server_addr, server_addr,
        sizeof(struct net_addr));
}