#ifndef P4IOEMU_DEVICE_H
#define P4IOEMU_DEVICE_H

#include <windows.h>

#include <stdint.h>
#include <stdbool.h>

#include "hook/iohook.h"

struct p4ioemu_device_msg_hook {
    void (*jamma2_read)(void* resp, uint32_t nbytes);
    uint32_t (*command_handle)(uint8_t cmd, const void* payload,
        uint32_t payload_len, void* resp, uint32_t resp_max_len);
    void (*roundplug_read_id)(uint8_t type, void* buffer, uint32_t len);
    void (*roundplug_read_mem)(uint8_t type, void* buffer, uint32_t len);
};

void p4ioemu_init(const struct p4ioemu_device_msg_hook* msg_hook);
void p4ioemu_fini(void);
HRESULT p4ioemu_dispatch_irp(struct irp *irp);

#endif
