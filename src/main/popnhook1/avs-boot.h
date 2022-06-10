#ifndef POPNHOOK1_AVS_BOOT_H
#define POPNHOOK1_AVS_BOOT_H

#include "util/net.h"

/**
 * Initialize hooking of avs_boot and ea3_boot. This re-enables avs logging
 * and injects a few important settings.
 */
void popnhook1_avs_boot_init();

/**
 * Set the target eamuse server address.
 *
 * @param server_addr Address to target eamuse server.
 */
void popnhook1_avs_boot_set_eamuse_addr(const struct net_addr *server_addr);

#endif
