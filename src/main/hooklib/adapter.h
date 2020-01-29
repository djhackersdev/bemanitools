#ifndef HOOKLIB_ADAPTER_H
#define HOOKLIB_ADAPTER_H

/**
 * Hooks GetAdaptersInfo to attempt only returning 1 adapter.
 *
 */
void adapter_hook_init(void);

/**
 * Uses the provided address to try and match a network adapter.
 * This adapter is then returned as the only adapter.
 *
 * @param network ip to match
 */
void adapter_hook_override(const char *adapter_address);

#endif
