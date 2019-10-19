#ifndef EZUSB_IIDX_MSG_H
#define EZUSB_IIDX_MSG_H

#include <stddef.h>
#include <stdint.h>

#define EZUSB_PAGESIZE 62

enum ezusb_iidx_msg_pipe {
    EZUSB_IIDX_MSG_PIPE_INTERRUPT_OUT = 0,
    EZUSB_IIDX_MSG_PIPE_INTERRUPT_IN = 1,
    EZUSB_IIDX_MSG_PIPE_BULK_OUT = 2,
    EZUSB_IIDX_MSG_PIPE_BULK_IN = 3
};

enum ezusb_iidx_msg_node {
    EZUSB_IIDX_MSG_NODE_NONE = 0x00,
    EZUSB_IIDX_MSG_NODE_SECURITY_PLUG = 0x01,
    EZUSB_IIDX_MSG_NODE_EEPROM = 0x02,
    EZUSB_IIDX_MSG_NODE_SERIAL = 0x04,
    EZUSB_IIDX_MSG_NODE_FPGA_V2 = 0x04,
    EZUSB_IIDX_MSG_NODE_16SEG = 0x05,
    EZUSB_IIDX_MSG_NODE_COIN = 0x09,
    EZUSB_IIDX_MSG_NODE_FPGA_V1 = 0x10,
    EZUSB_IIDX_MSG_NODE_WDT = 0x0C,
    EZUSB_IIDX_MSG_NODE_SRAM = 0x40,
    EZUSB_IIDX_MSG_NODE_SECURITY_MEM = 0xFE,
};

struct ezusb_iidx_msg_bulk_packet {
    uint8_t node;
    uint8_t page;
    uint8_t payload[EZUSB_PAGESIZE];
};

struct ezusb_iidx_msg_interrupt_write_packet {
    uint16_t deck_lights;
    uint8_t node;
    uint8_t cmd;
    uint8_t cmd_detail[2];
    uint8_t panel_lights;
    uint8_t unk0;
    uint8_t top_lamps;
    uint8_t top_neons;
    /* Ensure this is always enabled (1) after flashing the fpga prog.
       Otherwise, all data coming from the fpga is never updated */
    uint8_t fpga_run;
    uint8_t unk2;
    uint8_t unk3;
    uint8_t unk4;
    uint8_t unk5;
    uint8_t unk6;
};

struct ezusb_iidx_msg_interrupt_read_packet {
    /*
      Dip switches somewhere here?
      0: Not used
      1: Not used
      2: Not used
      3: Not used
      4: Not used
      5: Not used
      6: usb mute?
      7: Not used

      8: P1_1
      9: P1_2
      10: P1_3
      11: P1_4
      12: P1_5
      13: P1_6
      14: P1_7
      15: P2_1

      16: P2_2
      17: P2_3
      18: P2_4
      19: P2_5
      20: P2_6
      21: P2_7
      22: Coin mech
      23: Not used
      24: P1_Start

      25: P2_Start
      26: VEFX
      27: Effector
      28: Test
      29: Service
      30: unknown/not used
      31: coin mode state: this needs to be set according to the currently
          active coin node (coin mode1 -> 0, coin mode 2 -> 1)
    */
    uint32_t inverted_pad;
    uint8_t status;
    uint8_t unk0;
    uint8_t unk1;
    uint8_t p2_turntable;
    uint8_t p1_turntable;
    uint8_t seq_no;
    uint8_t fpga2_check_flag_unkn;
    uint8_t fpga_write_ready;
    uint8_t serial_io_busy_flag;
    uint8_t sliders[3];
};

#endif
