#define LOG_MODULE "acioemu-emu"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "acioemu/pipe.h"

#include "util/defs.h"
#include "util/list.h"
#include "util/mem.h"
#include "util/time.h"

static struct ac_io_in_queued *
ac_io_in_queued_alloc(struct ac_io_in *in, uint64_t delay_us);
static void ac_io_in_queued_populate(
    struct ac_io_in_queued *iq, const struct ac_io_message *msg);
static void ac_io_in_queued_putc(struct ac_io_in_queued *iq, uint8_t b);

static bool ac_io_out_supply_byte(struct ac_io_out *out, uint8_t b);
static bool ac_io_out_supply_frame_byte(struct ac_io_out *out, uint8_t b);
static bool ac_io_out_detect_broadcast_eof(struct ac_io_out *out);
static bool ac_io_out_detect_command_eof(struct ac_io_out *out);
static bool ac_io_out_check_sum(struct ac_io_out *out);
static bool ac_io_out_accept_message(struct ac_io_out *out);
static bool ac_io_out_reject_message(struct ac_io_out *out);

static bool ac_io_enable_legacy_mode = false;

static struct ac_io_in_queued *
ac_io_in_queued_alloc(struct ac_io_in *in, uint64_t delay_us)
{
    struct ac_io_in_queued *iq;

    /* TODO ordered insert by scheduled_time */

    iq = xmalloc(sizeof(*iq));
    iq->iobuf.bytes = iq->bytes;
    iq->iobuf.nbytes = 1;
    iq->iobuf.pos = 0;
    iq->scheduled_time = time_get_counter();
    iq->delay_us = delay_us;
    iq->bytes[0] = AC_IO_SOF;

    list_append(&in->queue, &iq->node);

    return iq;
}

static void ac_io_in_queued_populate(
    struct ac_io_in_queued *iq, const struct ac_io_message *msg)
{
    uint8_t checksum;
    const uint8_t *src;
    size_t nbytes;
    size_t i;

    if (msg->addr == AC_IO_BROADCAST) {
        nbytes = offsetof(struct ac_io_message, bcast.raw) + msg->bcast.nbytes;
    } else {
        nbytes = offsetof(struct ac_io_message, cmd.raw) + msg->cmd.nbytes;
    }

    src = (const uint8_t *) msg;
    checksum = 0;

    for (i = 0; i < nbytes; i++) {
        ac_io_in_queued_putc(iq, src[i]);
        checksum += src[i];
    }

    ac_io_in_queued_putc(iq, checksum);
}

static void ac_io_in_queued_putc(struct ac_io_in_queued *iq, uint8_t b)
{
    if (b == AC_IO_SOF || b == AC_IO_ESCAPE) {
        iq->bytes[iq->iobuf.nbytes++] = AC_IO_ESCAPE;
        iq->bytes[iq->iobuf.nbytes++] = ~b;
    } else {
        iq->bytes[iq->iobuf.nbytes++] = b;
    }
}

void ac_io_legacy_mode(void)
{
    log_info("Running acioemu legacy mode");
    ac_io_enable_legacy_mode = true;
}

void ac_io_in_init(struct ac_io_in *in)
{
    list_init(&in->queue);
}

void ac_io_in_supply(
    struct ac_io_in *in, const struct ac_io_message *msg, uint64_t delay_us)
{
    struct ac_io_in_queued *dest;

    dest = ac_io_in_queued_alloc(in, delay_us);
    dest->thunk = NULL;

    if (msg == NULL) {
        return;
    }

    ac_io_in_queued_populate(dest, msg);
}

void ac_io_in_supply_thunk(
    struct ac_io_in *in, ac_io_in_thunk_t thunk, void *ctx, uint64_t delay_us)
{
    struct ac_io_in_queued *dest;

    dest = ac_io_in_queued_alloc(in, delay_us);

    dest->thunk = thunk;
    dest->thunk_ctx = ctx;
}

void ac_io_in_drain(struct ac_io_in *in, struct iobuf *dest)
{
    struct ac_io_in_queued *iq;
    struct list_node *node;
    struct ac_io_message msg;
    size_t nmoved;
    uint64_t now;
    uint64_t elapsed_us;

    now = time_get_counter();

    do {
        node = list_peek_head(&in->queue);

        if (node == NULL) {
            break;
        }

        iq = containerof(node, struct ac_io_in_queued, node);
        elapsed_us = time_get_elapsed_us(
            now > iq->scheduled_time ? now - iq->scheduled_time : 0);

        if (elapsed_us < iq->delay_us) {
            break;
        }

        if (iq->thunk != NULL) {
            iq->thunk(iq->thunk_ctx, &msg);
            iq->thunk = NULL;

            ac_io_in_queued_populate(iq, &msg);
        }

        nmoved = iobuf_move(dest, &iq->iobuf);

        if (iq->iobuf.pos == iq->iobuf.nbytes) {
            list_pop_head(&in->queue);
            free(iq);
        }

        /* Legacy mode for old libacio versions like DistorteD:
           spit out single responses instead of combining them into a single
           io buffer. Some old libacio versions expect separate messages and
           will error (code 0x00000002) on multiple messages in a single buffer
           because they just read the first message and drop the remaining ones.
           If using legacy mode on newer libacio versions (e.g. Copula),
           the game will error with the same error code.
        */
        if (ac_io_enable_legacy_mode) {
            break;
        }
    } while (nmoved > 0);
}

bool ac_io_in_is_msg_pending(const struct ac_io_in *in)
{
    return list_peek_head_const(&in->queue) != NULL;
}

void ac_io_out_init(struct ac_io_out *out)
{
    out->pos = 0;
    out->in_frame = false;
    out->escape = false;
    out->have_message = false;
}

void ac_io_out_supply(struct ac_io_out *out, struct const_iobuf *src)
{
    log_assert(!ac_io_out_have_message(out));

    while (src->pos < src->nbytes) {
        if (!ac_io_out_supply_byte(out, src->bytes[src->pos++])) {
            break;
        }
    }
}

static bool ac_io_out_supply_byte(struct ac_io_out *out, uint8_t b)
{
    if (out->in_frame) {
        return ac_io_out_supply_frame_byte(out, b);
    } else {
        if (b == AC_IO_SOF) {
            out->in_frame = true;
        } else if (b == AC_IO_ESCAPE) {
            log_warning("Framing error: Escape byte outside frame");
        }

        return true;
    }
}

static bool ac_io_out_supply_frame_byte(struct ac_io_out *out, uint8_t b)
{
    if (out->escape) {
        out->escape = false;

        if (b == AC_IO_SOF || b == AC_IO_ESCAPE) {
            log_warning("Framing error: Control byte after escape byte");
        }

        b = ~b;
    } else if (b == AC_IO_ESCAPE) {
        out->escape = true;

        return true;
    } else if (b == AC_IO_SOF) {
        if (out->pos == 0) {
            /* Got autobaud/empty message */
            out->have_message = true;

            return false;
        } else {
            log_warning("Truncated message");
            out->pos = 0;

            return true;
        }
    }

    /* Payload byte */

    out->bytes[out->pos++] = b;

    /* Handle contextually-implied end-of-packet events */

    if (out->pos > offsetof(struct ac_io_message, addr)) {
        if (out->msg.addr == AC_IO_BROADCAST) {
            return ac_io_out_detect_broadcast_eof(out);
        } else {
            return ac_io_out_detect_command_eof(out);
        }
    }

    return true;
}

static bool ac_io_out_detect_broadcast_eof(struct ac_io_out *out)
{
    size_t end;

    if (out->pos > offsetof(struct ac_io_message, bcast.nbytes)) {
        end = offsetof(struct ac_io_message, bcast.raw) +
            out->msg.bcast.nbytes + 1;

        if (out->pos == end) {
            return ac_io_out_check_sum(out);
        }
    }

    return true;
}

static bool ac_io_out_detect_command_eof(struct ac_io_out *out)
{
    size_t end;

    if (out->pos > offsetof(struct ac_io_message, cmd.nbytes)) {
        end = offsetof(struct ac_io_message, cmd.raw) + out->msg.cmd.nbytes + 1;

        if (out->pos == end) {
            return ac_io_out_check_sum(out);
        }
    }

    return true;
}

static bool ac_io_out_check_sum(struct ac_io_out *out)
{
    uint8_t checksum;
    size_t i;

    checksum = 0;

    for (i = 0; i < out->pos - 1; i++) {
        checksum += out->bytes[i];
    }

    if (checksum == out->bytes[out->pos - 1]) {
        return ac_io_out_accept_message(out);
    } else {
        log_warning(
            "Checksum bad: expected %02x got %02x",
            checksum,
            out->bytes[out->pos - 1]);

        return ac_io_out_reject_message(out);
    }
}

static bool ac_io_out_accept_message(struct ac_io_out *out)
{
    out->have_message = true;

    return false;
}

static bool ac_io_out_reject_message(struct ac_io_out *out)
{
    ac_io_out_consume_message(out);

    return true;
}

bool ac_io_out_have_message(const struct ac_io_out *out)
{
    return out->have_message;
}

const struct ac_io_message *ac_io_out_get_message(const struct ac_io_out *out)
{
    if (out->pos == 0) {
        /* Autobaud byte/empty frame */
        return NULL;
    } else {
        return &out->msg;
    }
}

void ac_io_out_consume_message(struct ac_io_out *out)
{
    if (out->pos != 0) {
        out->in_frame = false;
    }

    out->pos = 0;
    out->have_message = false;
    out->escape = false;
}
