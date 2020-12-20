#define LOG_MODULE "iidx-bio2-exit-hook"

#include <windows.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "acio/acio.h"

#include "bio2/bi2a-iidx.h"
#include "bio2drv/detect.h"

#include "hook/iobuf.h"
#include "hook/iohook.h"

#include "util/log.h"
#include "util/mem.h"
#include "util/proc.h"
#include "util/str.h"

static const size_t _SIZE_ACIO_WRITE_SOF = sizeof(uint8_t);
// See struct ac_io_message: addr + code + seq_no + nbytes
static const size_t _SIZE_ACIO_WRITE_MSG_HEADER = 
        sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t) * 2;
static const uint8_t _ACIO_NODE_BIO2 = 0x01;

static wchar_t _device_path[MAX_PATH];
static HANDLE _device_handle;

static struct iobuf _buffer;
static struct iobuf _buffer_unescaped;

static void _unescape_acio_buffer(struct iobuf* in, struct iobuf* out)
{
    log_assert(in);
    log_assert(out);

    bool next_escaped;

    next_escaped = false;

    for (size_t i = 0; i < in->pos; i++) {
        uint8_t b = in->bytes[i];

        if (next_escaped) {
            out->bytes[out->pos++] = ~b;
            next_escaped = false;
        } else {
            if (b == AC_IO_SOF) {
                continue;
            }

            if (b == AC_IO_ESCAPE) {
                next_escaped = true;
                continue;
            }

            out->bytes[out->pos++] = b;
        }
    }
}

static void _switch_off_all_lights(struct bi2a_iidx_state_out *state_out)
{
    log_assert(state_out);

    for (uint8_t i = 0; i < sizeof(state_out->PANEL); i++) {
        state_out->PANEL[i].l_state = 0;
    }
    
    for (uint8_t i = 0; i < sizeof(state_out->DECKSW); i++) {
        state_out->DECKSW[i].l_state = 0;
    }

    for (uint8_t i = 0; i < sizeof(state_out->SEG16); i++) {
        state_out->SEG16[i] = ' ';
    }

    for (uint8_t i = 0; i < sizeof(state_out->SPOTLIGHT1); i++) {
        state_out->SPOTLIGHT1[i].l_state = 0;
    }

    state_out->NEONLAMP.l_state = 0;

    for (uint8_t i = 0; i < sizeof(state_out->SPOTLIGHT2); i++) {
        state_out->SPOTLIGHT2[i].l_state = 0;
    }
}

static HRESULT _iohook_handler(struct irp *irp)
{
    HRESULT result;

    if (irp->op == IRP_OP_OPEN && _device_handle == INVALID_HANDLE_VALUE) {

        if (!wcscmp(irp->open_filename, _device_path)) {
            result = iohook_invoke_next(irp);

            if (result == S_OK) {
                _device_handle = irp->fd;
            }

            return result;
        } else {
            return iohook_invoke_next(irp);
        }
    } else if (irp->fd == _device_handle) {
        switch (irp->op) {
            case IRP_OP_READ:
                result = iohook_invoke_next(irp);
                
                if (result != S_OK) {
                    return result;
                }

                size_t to_read = irp->read.nbytes;

                if (_buffer.pos + to_read >= _buffer.nbytes) {
                    log_warning("Read buffer size exceeded");
                    to_read = _buffer.nbytes - _buffer.pos;
                }

                memcpy(_buffer.bytes + _buffer.pos, irp->read.bytes, to_read);

                _buffer.pos += to_read;

                return result;

            case IRP_OP_WRITE:
                // Use write as a trigger to evaluate buffered data of previous reads.
                // Good enough to check for some button press combincation to exit the game
                // Not implementing a full acio stack for performance reasons
                // and to keep the complexity of this rather low for such crude feature.
         
                // Un-escape the buffer to prepare it for further evaluation
                _buffer_unescaped.pos = 0;
                _unescape_acio_buffer(&_buffer, &_buffer_unescaped);

                struct ac_io_message *msg = (struct ac_io_message*) _buffer_unescaped.bytes;

                if (msg->addr == (AC_IO_RESPONSE_FLAG | _ACIO_NODE_BIO2) && 
                        ac_io_u16(msg->cmd.code) == BIO2_BI2A_CMD_POLL) {
                    struct bi2a_iidx_state_in *state_in = (struct bi2a_iidx_state_in*) msg->cmd.raw;

                    if (state_in->PANEL.y_start1 && state_in->PANEL.y_start2 && 
                            state_in->PANEL.y_vefx && state_in->PANEL.y_effect) {
                        log_info("Exit hook triggered");

                        // Last opportunity to write some output data to switch off lights
                        // Hacky, since we don't have an acio stack but some checks to
                        // protect from bad things happening
                
                        if (irp->write.nbytes >=
                                _SIZE_ACIO_WRITE_SOF +
                                _SIZE_ACIO_WRITE_MSG_HEADER +
                                sizeof(struct bi2a_iidx_state_out)) {
                            log_misc("Switching off lights");

                            // +_SIZE_ACIO_WRITE_SOF: Skip leading 0xAA
                            struct ac_io_message *msg_out = 
                                (struct ac_io_message*) (irp->write.bytes + _SIZE_ACIO_WRITE_SOF);

                            // Guard assumption that reading poll is always followed by a writing
                            // poll (according to traffic dumps from iidx 27, that's the case)
                            if (msg_out->addr == _ACIO_NODE_BIO2 && 
                                    ac_io_u16(msg_out->cmd.code) == BIO2_BI2A_CMD_POLL &&
                                    msg_out->cmd.count == sizeof(struct bi2a_iidx_state_out)) {
                                struct bi2a_iidx_state_out *state_out =
                                    (struct bi2a_iidx_state_out*) msg_out->cmd.raw;

                                _switch_off_all_lights(state_out);

                                result = iohook_invoke_next(irp);

                                if (result != S_OK) {
                                    log_warning("Writing output to switch lights off failed: %lX",
                                        result);
                                }
                            } else {
                                log_warning("Skipping switching off lights, write output not "
                                    "identified as a poll message");
                            }
                        } else {
                            log_warning(
                                    "Skipped switching off lights due to insufficient buffer size");
                        }

                        Sleep(1000);
                        
                        /* Don't use ExitProcess. This might result in deadlocks
                           on newer games which rely more on multi threading */
                        proc_terminate_current_process(0);
                    }
                }

                _buffer.pos = 0;

                return iohook_invoke_next(irp);

            case IRP_OP_CLOSE:
                _device_handle = INVALID_HANDLE_VALUE;
                return iohook_invoke_next(irp);

            default:
                return iohook_invoke_next(irp);
        }
    } else {
        return iohook_invoke_next(irp);
    }
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    log_to_writer(log_writer_stdout, NULL);

    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    _device_handle = INVALID_HANDLE_VALUE;

    char buffer[MAX_PATH];

    if (!bio2drv_detect(
            DETECT_DEVICEID,
            0,
            buffer,
            sizeof(buffer))) {
        log_warning("Autodetecting IIDX BIO2 failed, disabling exit hook");
        return TRUE;
    }

    log_info("BIO2 device found: %s", buffer);

    wchar_t* buffer_widen = str_widen(buffer);

    wcscpy(_device_path, buffer_widen);
    free(buffer_widen);

    // should be enough for this use-case
    _buffer.nbytes = 1024;
    _buffer.pos = 0;
    _buffer.bytes = xmalloc(_buffer.nbytes);

    _buffer_unescaped.nbytes = 1024;
    _buffer_unescaped.pos = 0;
    _buffer_unescaped.bytes = xmalloc(_buffer_unescaped.nbytes);

    iohook_push_handler(_iohook_handler);

    log_info("Initialized");

    return TRUE;
}
