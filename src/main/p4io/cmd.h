#ifndef P4IO_CMD_H
#define P4IO_CMD_H

#include <stdint.h>

#define P4IO_SOF 0xAA

enum p4ioemu_p4io_command {
    P4IO_CMD_INIT = 0x00,
    P4IO_CMD_GET_DEVICE_INFO = 0x01,
    P4IO_CMD_SET_PORTOUT = 0x12,
    P4IO_CMD_COINSTOCK = 0x18,
    P4IO_CMD_RESET_WTD = 0x1C,
    P4IO_CMD_SCI_MNG_OPEN = 0x20,
    P4IO_CMD_SCI_UPDATE = 0x21,
    /* SCI = serial communication interface */
    P4IO_CMD_SCI_MNG_BREAK = 0x24,
    /* Read round plug id over one-wire */
    P4IO_CMD_DALLAS_READ_ID = 0x40,
    /* Read round plug mem over one-wire */
    P4IO_CMD_DALLAS_READ_MEM = 0x41
};

#define P4IO_CMD_HEADER_LEN 4
#define P4IO_MAX_PAYLOAD 60

struct p4io_cmd_header {
    uint8_t sof;
    uint8_t cmd;
    uint8_t seq_num;
    uint8_t payload_len;
};

struct p4io_cmd_package {
    struct p4io_cmd_header header;
    uint8_t payload[P4IO_MAX_PAYLOAD];
};

struct p4io_req_read_roundplug {
    /* 0 = black, 1 = white */
    uint8_t type;
};

struct p4io_req_panel_mode {
    uint16_t mode;
    uint8_t is_single;
    uint8_t padding[13];
} __attribute__((__packed__));

struct p4io_resp_device_info {
    uint32_t type;
    uint8_t padding;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_revision;
    char product_code[4];
    char build_date[16];
    char build_time[16];
};


#endif
