#ifndef EZUSB_IIDX_FPGA_CMD_H
#define EZUSB_IIDX_FPGA_CMD_H

enum ezusb_iidx_fpga_cmd_v1 {
    EZUSB_IIDX_FPGA_CMD_V1_INIT = 0x01,
    // TODO rename to reset
    EZUSB_IIDX_FPGA_CMD_V1_CHECK = 0xFF,
    // TODO rename to check? -> see firmware
    EZUSB_IIDX_FPGA_CMD_V1_CHECK_2 = 0x02,
    EZUSB_IIDX_FPGA_CMD_V1_WRITE = 0x03,
    EZUSB_IIDX_FPGA_CMD_V1_WRITE_DONE = 0x04
};

enum ezusb_iidx_fpga_cmd_v2 {
    EZUSB_IIDX_FPGA_CMD_V2_INIT = 0x01,
    EZUSB_IIDX_FPGA_CMD_V2_CHECK = 0x02,
    EZUSB_IIDX_FPGA_CMD_V2_WRITE = 0x03,
    EZUSB_IIDX_FPGA_CMD_V2_WRITE_DONE = 0x04
};

enum ezusb_iidx_fpga_cmd_status_v1 {
    EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK = 0x00,
    /* I even checked the firmware and they used the same
       return code for both, error and ok on some calls...*/
    EZUSB_IIDX_FPGA_CMD_STATUS_V1_OK_2 = 0xFE,
    EZUSB_IIDX_FPGA_CMD_STATUS_V1_FAULT = 0xFE,
};

enum ezusb_iidx_fpga_cmd_status_v2 {
    EZUSB_IIDX_FPGA_CMD_STATUS_V2_NULL = 0x00,
    EZUSB_IIDX_FPGA_CMD_STATUS_V2_INIT_OK = 0x41,
    EZUSB_IIDX_FPGA_CMD_STATUS_V2_CHECK_OK = 0x42,
    EZUSB_IIDX_FPGA_CMD_STATUS_V2_WRITE_OK = 0x43,
    EZUSB_IIDX_FPGA_CMD_STATUS_V2_FAULT = 0xFE
};

#endif