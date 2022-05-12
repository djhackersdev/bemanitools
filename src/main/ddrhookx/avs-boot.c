#define LOG_MODULE "avs-boot"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hook/iohook.h"
#include "hook/table.h"

#include "imports/avs.h"

#include "ddrhookx/avs-boot.h"
#include "ddrhookx/filesystem.h"

#include "util/log.h"
#include "util/str.h"

static void (*real_avs_boot)(
    struct property_node *config,
    void *std_heap,
    size_t sz_std_heap,
    void *avs_heap,
    size_t sz_avs_heap,
    avs_log_writer_t log_writer,
    void *log_context);
static int (*real_ea3_boot)(struct property_node *config);

static int my_ea3_boot(struct property_node *config);

static void my_avs_boot(
    struct property_node *config,
    void *std_heap,
    size_t sz_std_heap,
    void *avs_heap,
    size_t sz_avs_heap,
    avs_log_writer_t log_writer,
    void *log_context);
static struct net_addr ddrhookx_avs_boot_eamuse_server_addr;

static const struct hook_symbol ddrxhook_avs_hook_syms[] = {
    {.name = "avs_boot",
     .patch = my_avs_boot,
     .link = (void **) &real_avs_boot},
};

static const struct hook_symbol ddrhookx_avs_ea3_hook_syms[] = {
    {.name = "ea3_boot",
     .patch = my_ea3_boot,
     .link = (void **) &real_ea3_boot},
};

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

    char nvram_path[MAX_PATH] = {0};
    char raw_path[MAX_PATH] = {0};

    // Using the full path as part of the AVS paths seems to break on long paths.
    // So instead, just take the launcher folder relative to the main game directory.
    char *launcher_folder;
    get_launcher_path_parts(NULL, &launcher_folder);

    if (launcher_folder) {
        strcpy(nvram_path, launcher_folder);
        strcat(nvram_path, "\\");

        strcpy(raw_path, launcher_folder);
        strcat(raw_path, "\\");

        free(launcher_folder);
    }

    strcat(nvram_path, "conf\\nvram");
    strcat(raw_path, "conf\\raw");

    log_misc("avs paths: %s %s\n", nvram_path, raw_path);

    avs_boot_replace_property_str(
        config, "/fs/nvram/device", nvram_path);
    avs_boot_replace_property_str(
        config, "/fs/raw/device", raw_path);

    real_avs_boot(
        config,
        std_heap,
        sz_std_heap,
        avs_heap,
        sz_avs_heap,
        log_writer_debug,
        NULL);
}

static int my_ea3_boot(struct property_node *config)
{
    char *server_addr;

    log_info("Called my_ea3_boot");

    if (ddrhookx_avs_boot_eamuse_server_addr.type != NET_ADDR_TYPE_INVALID) {
        log_misc("Injecting network server address");

        server_addr = net_addr_to_str(&ddrhookx_avs_boot_eamuse_server_addr);

        avs_boot_replace_property_str(config, "network/services", server_addr);

        free(server_addr);
    }

    return real_ea3_boot(config);
}

void ddrhookx_avs_boot_init()
{
    hook_table_apply(
        NULL,
        "libavs-win32.dll",
        ddrxhook_avs_hook_syms,
        lengthof(ddrxhook_avs_hook_syms));

    hook_table_apply(
        NULL,
        "libavs-win32-ea3.dll",
        ddrhookx_avs_ea3_hook_syms,
        lengthof(ddrhookx_avs_ea3_hook_syms));

    memset(&ddrhookx_avs_boot_eamuse_server_addr, 0, sizeof(struct net_addr));

    log_info("Inserted avs log hooks");
}

void ddrhookx_avs_boot_set_eamuse_addr(const struct net_addr *server_addr)
{
    char *str;

    str = net_addr_to_str(server_addr);
    log_info("Setting eamuse server: %s", str);
    free(str);

    memcpy(
        &ddrhookx_avs_boot_eamuse_server_addr,
        server_addr,
        sizeof(struct net_addr));
}