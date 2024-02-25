#define LOG_MODULE "avs-boot"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "core/log.h"

#include "hook/table.h"

#include "imports/avs.h"

#include "popnhook1/avs-boot.h"

static int (*real_ea3_boot_avs)(struct property_node *config);
static int (*real_ea3_boot)(struct property_node *config);

static int my_ea3_boot_avs(struct property_node *config);
static int my_ea3_boot(struct property_node *config);

static struct net_addr popnhook1_avs_boot_eamuse_server_addr;

static const struct hook_symbol popnhook1_avs_hook_syms[] = {
    {.name = "ea3_boot",
     .patch = my_ea3_boot_avs,
     .link = (void **) &real_ea3_boot_avs},
};

static const struct hook_symbol popnhook1_ea3_hook_syms[] = {
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

static void insert_eamuse_addr(struct property_node *config)
{
    char *server_addr;

    if (popnhook1_avs_boot_eamuse_server_addr.type != NET_ADDR_TYPE_INVALID) {
        log_misc("Injecting network server address");

        server_addr = net_addr_to_str(&popnhook1_avs_boot_eamuse_server_addr);

        avs_boot_replace_property_str(config, "network/services", server_addr);

        free(server_addr);
    }
}

static int my_ea3_boot(struct property_node *config)
{
    log_info("Called my_ea3_boot");
    insert_eamuse_addr(config);
    return real_ea3_boot(config);
}

static int my_ea3_boot_avs(struct property_node *config)
{
    log_info("Called my_ea3_boot_avs");
    insert_eamuse_addr(config);
    return real_ea3_boot_avs(config);
}

void popnhook1_avs_boot_init()
{
    // pop'n 15 has the ea3_boot in libavs-win32.dll
    hook_table_apply(
        NULL,
        "libavs-win32.dll",
        popnhook1_avs_hook_syms,
        lengthof(popnhook1_avs_hook_syms));

    hook_table_apply(
        NULL,
        "libavs-win32-ea3.dll",
        popnhook1_ea3_hook_syms,
        lengthof(popnhook1_ea3_hook_syms));

    memset(&popnhook1_avs_boot_eamuse_server_addr, 0, sizeof(struct net_addr));

    log_info("Inserted avs log hooks");
}

void popnhook1_avs_boot_set_eamuse_addr(const struct net_addr *server_addr)
{
    char *str;

    str = net_addr_to_str(server_addr);
    log_info("Setting eamuse server: %s", str);
    free(str);

    memcpy(
        &popnhook1_avs_boot_eamuse_server_addr,
        server_addr,
        sizeof(struct net_addr));
}