#define LOG_MODULE "ezusb2-iidx"

#include "ezusb2/ezusb2.h"

#include "msg.h"

enum ezusb2_iidx_msg_pipe {
    EZUSB2_IIDX_MSG_PIPE_INTERRUPT_OUT = 0x01,
    EZUSB2_IIDX_MSG_PIPE_BULK_OUT = 0x02,

    EZUSB2_IIDX_MSG_PIPE_INTERRUPT_IN = 0x81,
    /* yes, 0x86 and NOT 0x82... */
    EZUSB2_IIDX_MSG_PIPE_BULK_IN = 0x86
};

bool ezusb2_iidx_interrupt_read(HANDLE handle,
        struct ezusb2_iidx_msg_interrupt_read_packet* packet)
{
    return ezusb2_endpoint_transfer(handle, EZUSB2_IIDX_MSG_PIPE_INTERRUPT_IN, 
        (void*) packet, sizeof(struct ezusb2_iidx_msg_interrupt_read_packet));
}

bool ezusb2_iidx_interrupt_write(HANDLE handle,
        const struct ezusb2_iidx_msg_interrupt_write_packet* packet)
{
    return ezusb2_endpoint_transfer(handle, EZUSB2_IIDX_MSG_PIPE_INTERRUPT_OUT, 
        (void*) packet, sizeof(struct ezusb2_iidx_msg_interrupt_write_packet));
}