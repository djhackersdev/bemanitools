#ifndef EZUSB_IIDX_H
#define EZUSB_IIDX_H

#include <stdbool.h>

#include <windows.h>

#include "msg.h"

bool ezusb_iidx_ioctl(HANDLE handle, uint32_t code, void* in_bytes, 
    uint32_t in_nbytes, void* out_bytes, uint32_t out_nbytes, 
    uint32_t* out_returned);

bool ezusb_iidx_interrupt_read(HANDLE handle,
    struct ezusb_iidx_msg_interrupt_read_packet* packet);

bool ezusb_iidx_interrupt_write(HANDLE handle,
    const struct ezusb_iidx_msg_interrupt_write_packet* packet);

bool ezusb_iidx_bulk_read(HANDLE handle,
    struct ezusb_iidx_msg_bulk_packet* packet);

bool ezusb_iidx_bulk_write(HANDLE handle,
    const struct ezusb_iidx_msg_bulk_packet* packet);

bool ezusb_iidx_execute_cmd(HANDLE handle, uint8_t node,
    uint8_t cmd, uint8_t cmd_detail, uint8_t cmd_detail2,
    struct ezusb_iidx_msg_interrupt_read_packet* int_read_data);

uint8_t ezusb_iidx_execute_cmd_retry(HANDLE handle, uint8_t node,
    uint8_t cmd, uint8_t cmd_detail, uint8_t cmd_detail2, uint8_t int_reads);

bool ezusb_iidx_execute_cmd_timeout(HANDLE handle, uint8_t node,
    uint8_t cmd, uint8_t cmd_detail, uint8_t cmd_detail2,
    uint8_t expected_ret_code, uint32_t timeout_ms);

#endif