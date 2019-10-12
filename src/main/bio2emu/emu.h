#ifndef BIO2EMU_EMU_H
#define BIO2EMU_EMU_H

#include "hook/iohook.h"
#include "acioemu/emu.h"

struct bio2emu_port;
struct ac_io_message;

typedef void (*bio2_bi2a_dispatcher)(struct bio2emu_port *emu, const struct ac_io_message *req);

struct bio2emu_port {
    struct ac_io_emu acio;
    const char* port;
    const wchar_t* wport;
    bio2_bi2a_dispatcher dispatcher;
};


void bio2emu_init();

void bio2emu_port_init(struct bio2emu_port* bio2emu_emu);
void bio2emu_port_fini(struct bio2emu_port* bio2emu_emu);

HRESULT bio2emu_port_dispatch_irp(struct irp *irp);

#endif
