#ifndef P3IO_CMD_H
#define P3IO_CMD_H

#include <stdint.h>

#include "p3io/frame.h"

// Max message size of bulk endpoint for p3io command messaging
#define P3IO_MAX_MESSAGE_SIZE 256

enum {
    P3IO_CMD_GET_VERSION = 0x01,
    // Appears to be some alternative "initialize" (set mode) command call on
    // DDR but currently unclear how this is being used. So far, it causes the
    // p3io (on a SD cab) to get stuck/crash when issued.
    P3IO_CMD_INIT2 = 0x03,
    P3IO_CMD_SET_WATCHDOG = 0x05,
    P3IO_CMD_POWEROFF = 0x22,
    P3IO_CMD_SET_OUTPUTS = 0x24,
    P3IO_CMD_READ_PLUG = 0x25,
    P3IO_CMD_GET_CAB_TYPE_OR_DIPSW = 0x27,
    P3IO_CMD_GET_VIDEO_FREQ = 0x29,
    // Unknown command that is part of DDR's IO setup routine
    P3IO_CMD_UNKNOWN_2B = 0x2B,
    P3IO_CMD_INIT = 0x2F,
    P3IO_CMD_GET_COINSTOCK = 0x31,
    P3IO_CMD_SET_COINCOUNTER = 0x32,
    P3IO_CMD_RS232_OPEN_CLOSE = 0x38,
    P3IO_CMD_RS232_WRITE = 0x3A,
    P3IO_CMD_RS232_READ = 0x3B,
};

enum p3io_rs232_cmd {
    P3IO_RS232_CMD_OPEN = 0x00,
    P3IO_RS232_CMD_CLOSE = 0xFF,
};

enum p3io_rs232_baud_rate {
    P3IO_RS232_BAUD_19200 = 0x02,
    P3IO_RS232_BAUD_38400 = 0x03,
    P3IO_RS232_BAUD_57600 = 0x04,
};

/**
 * Selector for the P3IO_CMD_GET_CAB_TYPE_OR_DIPSW command
 */
enum p3io_cab_type_dip_sw_selector {
    P3IO_DIP_SW_SELECTOR = 0,
    P3IO_CAB_TYPE_SELECTOR = 1,
};

/**
 * Enum for available video frequencies to select from. Depending on the
 * game, this feature is used or unused.
 *
 * Set by a jumper wire on the Jamma harness.
 */
enum p3io_video_freq {
    P3IO_VIDEO_FREQ_15KHZ = 0xFF,
    P3IO_VIDEO_FREQ_31KHZ = 0x7F,
};

/**
 * Enum for available cabinet/display types to select from. Depending on the
 * game, this feature is used or unused.
 *
 * The p3io detects the monitor type through its monitor passthrough. If the
 * monitor provides an EDID, it assumes it's an HD monitor. Otherwise, it
 * assumes SD because CRT monitors likely do not provide EDIDs.
 */
enum p3io_cab_type {
    P3IO_CAB_TYPE_SD = 0,
    P3IO_CAB_TYPE_HD = 1,
};

#pragma pack(push, 1)

/**
 * p3io command message header of any message (request and response)
 */
struct p3io_hdr {
    uint8_t nbytes;
    uint8_t seq_no;
    uint8_t cmd;
};

// -----------------------------------------------------------------------------

struct p3io_req_version {
    struct p3io_hdr hdr;
};

struct p3io_req_init2 {
    struct p3io_hdr hdr;
    uint8_t unknown_AA;
};

struct p3io_req_watchdog {
    struct p3io_hdr hdr;
    uint8_t enable;
};

struct p3io_req_set_outputs {
    struct p3io_hdr hdr;
    uint8_t unk_FF;
    uint32_t outputs;
};

struct p3io_req_read_plug {
    struct p3io_hdr hdr;
    uint8_t flags;
};

struct p3io_req_get_cab_type_or_dipsw {
    struct p3io_hdr hdr;
    uint8_t cab_type_or_dipsw;
};

struct p3io_req_get_video_freq {
    struct p3io_hdr hdr;
    uint8_t unknown_05;
};

struct p3io_req_unknown_2b {
    struct p3io_hdr hdr;
    // either 0 or 1 set by game (boolean value)
    // set to 0 for p3io dragon (SD cab?)
    uint8_t unknown;
};

struct p3io_req_init {
    struct p3io_hdr hdr;
};

struct p3io_req_coin_stock {
    struct p3io_hdr hdr;
};

struct p3io_req_set_coin_counter {
    struct p3io_hdr hdr;
    uint8_t coin_counter[2];
};

struct p3io_req_rs232_open_close {
    struct p3io_hdr hdr;
    uint8_t port_no;
    uint8_t subcmd;
    uint8_t baud_code;
};

struct p3io_req_rs232_read {
    struct p3io_hdr hdr;
    uint8_t port_no;
    uint8_t nbytes;
};

struct p3io_req_rs232_write {
    struct p3io_hdr hdr;
    uint8_t port_no;
    uint8_t nbytes;
    uint8_t bytes[128];
};

struct p3io_req_raw {
    uint8_t data[P3IO_MAX_MESSAGE_SIZE];
};

// Structs sorted in command ID order
union p3io_req_any {
    struct p3io_hdr hdr;
    struct p3io_req_version version;
    struct p3io_req_init2 init2;
    struct p3io_req_watchdog watchdog;
    struct p3io_req_set_outputs set_outputs;
    struct p3io_req_read_plug read_plug;
    struct p3io_req_get_cab_type_or_dipsw cab_type_or_dipsw;
    struct p3io_req_get_video_freq video_freq;
    struct p3io_req_unknown_2b unknown_2b;
    struct p3io_req_init init;
    struct p3io_req_coin_stock coin_stock;
    struct p3io_req_set_coin_counter set_coin_counter;
    struct p3io_req_rs232_open_close rs232_open_close;
    struct p3io_req_rs232_read rs232_read;
    struct p3io_req_rs232_write rs232_write;
    struct p3io_req_raw raw;
};

// -----------------------------------------------------------------------------

struct p3io_resp_version {
    struct p3io_hdr hdr;
    char str[4];
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};

struct p3io_resp_init2 {
    struct p3io_hdr hdr;
    uint8_t status;
};

struct p3io_resp_watchdog {
    struct p3io_hdr hdr;
    uint8_t state;
};

struct p3io_resp_set_outputs {
    struct p3io_hdr hdr;
    // Apparently always 0xFF on real hardware
    uint8_t unkn_FF;
    uint8_t unkn;
};

struct p3io_resp_read_plug {
    struct p3io_hdr hdr;
    uint8_t present;
    uint8_t rom[8];
    uint8_t eeprom[32];
};

struct p3io_resp_get_cab_type_or_dipsw {
    struct p3io_hdr hdr;
    uint8_t state;
};

struct p3io_resp_get_video_freq {
    struct p3io_hdr hdr;
    uint8_t video_freq;
};

struct p3io_resp_unknown_2b {
    struct p3io_hdr hdr;
    uint8_t unknown;
};

struct p3io_resp_init {
    struct p3io_hdr hdr;
    uint8_t status;
};

struct p3io_resp_set_coin_counter {
    struct p3io_hdr hdr;
    uint8_t status;
};

struct p3io_resp_coin_stock {
    struct p3io_hdr hdr;
    uint8_t error;
    uint16_t slots[2];
};

struct p3io_resp_rs232_open_close {
    struct p3io_hdr hdr;
    uint8_t status;
};

struct p3io_resp_rs232_read {
    struct p3io_hdr hdr;
    uint8_t nbytes;
    uint8_t bytes[126];
};

struct p3io_resp_rs232_write {
    struct p3io_hdr hdr;
    uint8_t nbytes;
};

struct p3io_resp_raw {
    uint8_t data[P3IO_MAX_MESSAGE_SIZE];
};

union p3io_resp_any {
    struct p3io_hdr hdr;
    struct p3io_resp_version version;
    struct p3io_resp_init2 init2;
    struct p3io_resp_watchdog watchdog;
    struct p3io_resp_set_outputs set_outputs;
    struct p3io_resp_read_plug read_plug;
    struct p3io_resp_get_cab_type_or_dipsw cab_type_or_dipsw;
    struct p3io_resp_get_video_freq video_freq;
    struct p3io_resp_unknown_2b unknown_2b;
    struct p3io_resp_init init;
    struct p3io_resp_coin_stock coin_stock;
    struct p3io_resp_set_coin_counter set_coin_counter;
    struct p3io_resp_rs232_open_close rs232_open_close;
    struct p3io_resp_rs232_read rs232_read;
    struct p3io_resp_rs232_write rs232_write;
    struct p3io_resp_raw raw;
};

// -----------------------------------------------------------------------------

#pragma pack(pop)

uint8_t p3io_get_full_req_size(const union p3io_req_any *req);

uint8_t p3io_get_full_resp_size(const union p3io_resp_any *resp);

void p3io_req_hdr_init(
    struct p3io_hdr *hdr, uint8_t seq_no, uint8_t cmd, size_t size);

void p3io_resp_hdr_init(
    struct p3io_hdr *resp_hdr, size_t nbytes, const struct p3io_hdr *req_hdr);

#endif
