#ifndef P3IO_CMD_H
#define P3IO_CMD_H

#include <stdint.h>

#include "p3io/frame.h"

enum {
    P3IO_CMD_GET_VERSION            = 0x01,
    P3IO_CMD_SET_WATCHDOG           = 0x05,
    P3IO_CMD_POWEROFF               = 0x22,
    P3IO_CMD_SET_OUTPUTS            = 0x24,
    P3IO_CMD_READ_PLUG              = 0x25,
    P3IO_CMD_GET_CAB_TYPE_OR_DIPSW  = 0x27,
    P3IO_CMD_GET_VIDEO_FREQ         = 0x29,
    P3IO_CMD_SET_MODE               = 0x2F,
    P3IO_CMD_GET_COINSTOCK          = 0x31,
    P3IO_CMD_SET_COINCOUNTER        = 0x32,
    P3IO_CMD_RS232_OPEN_CLOSE       = 0x38,
    P3IO_CMD_RS232_WRITE            = 0x3A,
    P3IO_CMD_RS232_READ             = 0x3B,
};

enum {
    P3IO_RS232_CMD_OPEN         = 0x00,
    P3IO_RS232_CMD_CLOSE        = 0xFF,
};

enum {
    P3IO_RS232_BAUD_19200       = 0x02,
    P3IO_RS232_BAUD_38400       = 0x03,
    P3IO_RS232_BAUD_57600       = 0x04,
};

#pragma pack(push, 1)

struct p3io_hdr {
    uint8_t nbytes;
    uint8_t seq_no;
};

struct p3io_req_u8 {
    struct p3io_hdr hdr;
    uint8_t cmd;
    uint8_t u8;
};

struct p3io_req_get_cab_type_or_dipsw {
    struct p3io_hdr hdr;
    uint8_t cmd;
    uint8_t cab_type_or_dipsw;
};

struct p3io_req_set_coin_counter {
    struct p3io_hdr hdr;
    uint8_t cmd;
    uint8_t coin_counter[2];
};

struct p3io_req_set_outputs {
    struct p3io_hdr hdr;
    uint8_t cmd;
    uint8_t unk_00;
    uint32_t outputs;
};

struct p3io_req_read_plug {
    struct p3io_hdr hdr;
    uint8_t cmd;
    uint8_t flags;
};

struct p3io_req_rs232_open_close {
    struct p3io_hdr hdr;
    uint8_t cmd;
    uint8_t port_no;
    uint8_t subcmd;
    uint8_t baud_code;
};

struct p3io_req_rs232_read {
    struct p3io_hdr hdr;
    uint8_t cmd;
    uint8_t port_no;
    uint8_t nbytes;
};

struct p3io_req_rs232_write {
    struct p3io_hdr hdr;
    uint8_t cmd;
    uint8_t port_no;
    uint8_t nbytes;
    uint8_t bytes[128];
};

union p3io_req_any {
    struct p3io_hdr hdr;
    struct p3io_req_u8 u8;
    struct p3io_req_get_cab_type_or_dipsw cab_type_or_dipsw;
    struct p3io_req_set_coin_counter set_coin_counter;
    struct p3io_req_set_outputs set_outputs;
    struct p3io_req_read_plug read_plug;
    struct p3io_req_rs232_open_close rs232_open_close;
    struct p3io_req_rs232_read rs232_read;
    struct p3io_req_rs232_write rs232_write;
    uint8_t raw[128];
};

struct p3io_resp_u8 {
    struct p3io_hdr hdr;
    uint8_t status;
    uint8_t u8;
};

struct p3io_resp_get_cab_type_or_dipsw {
    struct p3io_hdr hdr;
    uint8_t status;
};

struct p3io_resp_coin_stock {
    struct p3io_hdr hdr;
    uint8_t status;
    uint8_t error;
    uint16_t slots[2];
};

struct p3io_resp_read_plug {
    struct p3io_hdr hdr;
    uint8_t status;
    uint8_t present;
    uint8_t rom[8];
    uint8_t eeprom[32];
};

struct p3io_resp_rs232_read {
    struct p3io_hdr hdr;
    uint8_t status;
    uint8_t nbytes;
    uint8_t bytes[126];
};

struct p3io_resp_rs232_write {
    struct p3io_hdr hdr;
    uint8_t status;
    uint8_t nbytes;
};

struct p3io_resp_version {
    struct p3io_hdr hdr;
    uint8_t status;
    char str[4];
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
};

union p3io_resp_any {
    struct p3io_hdr hdr;
    struct p3io_resp_u8 u8;
    struct p3io_resp_get_cab_type_or_dipsw cab_type_or_dipsw;
    struct p3io_resp_coin_stock coin_stock;
    struct p3io_resp_read_plug read_plug;
    struct p3io_resp_rs232_read rs232_read;
    struct p3io_resp_rs232_write rs232_write;
    struct p3io_resp_version version;
};

#pragma pack(pop)

uint8_t p3io_req_cmd(const union p3io_req_any *src);

void p3io_resp_init(
        struct p3io_hdr *dest,
        size_t nbytes,
        const struct p3io_hdr *req);

#endif
