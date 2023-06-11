#ifndef P3IO_UART_H
#define P3IO_UART_H

#include "p3io/cmd.h"

void p3io_uart_set_path(size_t uart_no, const wchar_t *path);

void p3io_uart_cmd_open_close(
    const struct p3io_req_rs232_open_close *req,
    struct p3io_resp_rs232_open_close *resp);

void p3io_uart_cmd_read(
    const struct p3io_req_rs232_read *req, struct p3io_resp_rs232_read *resp);

void p3io_uart_cmd_write(
    const struct p3io_req_rs232_write *req, struct p3io_resp_rs232_write *resp);

#endif
