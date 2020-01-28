#ifndef HOOKLIB_ADAPTER_H
#define HOOKLIB_ADAPTER_H

void adapter_hook_init(void);

/**
 * Uses the provided address to try and match a network adapter
 *
 * @param network ip to match
 */
void adapter_hook_override(const char *adapter_address);

#endif
