#ifndef ACIOEMU_EMU_H
#define ACIOEMU_EMU_H

#include <windows.h>

#include <stdbool.h>
#include <wchar.h>

#include "acio/acio.h"

#include "acioemu/pipe.h"

#include "hook/iohook.h"

struct ac_io_emu {
    HANDLE fd;
    wchar_t *wfilename;
    char *filename;
    struct ac_io_in in;
    struct ac_io_out out;
};

void ac_io_emu_init(struct ac_io_emu *emu, const wchar_t *filename);
void ac_io_emu_fini(struct ac_io_emu *emu);
bool ac_io_emu_match_irp(const struct ac_io_emu *emu, const struct irp *irp);
HRESULT ac_io_emu_dispatch_irp(struct ac_io_emu *emu, struct irp *irp);
const struct ac_io_message *ac_io_emu_request_peek(const struct ac_io_emu *emu);
void ac_io_emu_request_pop(struct ac_io_emu *emu);
void ac_io_emu_response_push(
    struct ac_io_emu *emu, const struct ac_io_message *resp, uint64_t delay_ms);
void ac_io_emu_response_push_thunk(
    struct ac_io_emu *emu,
    ac_io_in_thunk_t thunk,
    void *ctx,
    uint64_t delay_ms);

#endif
