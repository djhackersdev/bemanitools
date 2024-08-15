#ifndef ACIOEMU_PIPE_H
#define ACIOEMU_PIPE_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "acio/acio.h"

#include "iface-core/log.h"

#include "util/iobuf.h"
#include "util/list.h"

/* This uses the USB convention where OUT and IN are from the host's (game's)
   perspective. So an OUT transaction comes in to us and vice versa.

   (I made those terms up, not Konami). */

typedef void (*ac_io_in_thunk_t)(void *ctx, struct ac_io_message *msg);

struct ac_io_in_queued {
    struct list_node node;
    struct const_iobuf iobuf;
    uint64_t scheduled_time;
    uint64_t delay_us;
    ac_io_in_thunk_t thunk;
    void *thunk_ctx;
    uint8_t bytes[sizeof(struct ac_io_message)];
};

struct ac_io_in {
    struct list queue;
};

struct ac_io_out {
    union {
        uint8_t bytes[sizeof(struct ac_io_message)];
        struct ac_io_message msg;
    };

    size_t pos;
    bool in_frame;
    bool escape;
    bool have_message;
};

void ac_io_legacy_mode(void);

void ac_io_in_init(struct ac_io_in *in);
void ac_io_in_supply(
    struct ac_io_in *in, const struct ac_io_message *msg, uint64_t delay);
void ac_io_in_supply_thunk(
    struct ac_io_in *in, ac_io_in_thunk_t thunk, void *ctx, uint64_t delay);
void ac_io_in_drain(struct ac_io_in *in, struct iobuf *dest);
bool ac_io_in_is_msg_pending(const struct ac_io_in *in);

void ac_io_out_init(struct ac_io_out *out);
void ac_io_out_supply(struct ac_io_out *out, struct const_iobuf *src);
bool ac_io_out_have_message(const struct ac_io_out *out);
const struct ac_io_message *ac_io_out_get_message(const struct ac_io_out *out);
void ac_io_out_consume_message(struct ac_io_out *out);

#endif
