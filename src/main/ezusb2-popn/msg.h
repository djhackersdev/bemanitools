#ifndef EZUSB2_POPN_MSG_H
#define EZUSB2_POPN_MSG_H

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)
struct ezusb2_popn_msg_interrupt_write_packet {
    uint8_t unk0;
    uint8_t unk1;
    uint8_t node;
    uint8_t cmd;
    uint8_t cmd_detail[2];
    uint8_t unknown[2];
    uint32_t lamp;
    /* Pad to 64 byte total size because the endpoint expects this buffer size.
       Requests are hanging on ioctl calls otherwise. */
    uint8_t unknown2[52];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ezusb2_popn_msg_interrupt_read_packet {
    /*
      Pad mappings:
      0: unknown/not used
      1: unknown/not used
      2: unknown/not used
      3: unknown/not used
      4: unknown/not used
      5: unknown/not used
      6: service
      7: test

      8: button 1
      9: button 2
      10: button 3
      11: button 4
      12: button 5
      13: button 6
      14: button 7
      15: button 8

      16: button 9
      17: unknown/not used
      18: unknown/not used
      19: unknown/not used
      20: unknown/not used
      21: unknown/not used
      22: coin
      23: unknown/not used

      24: dipsw 1
      25: dipsw 2
      26: dipsw 3
      27: dipsw 4
      28: unknown/not used
      29: unknown/not used
      30: unknown/not used
      31: unknown/not used
    */
    uint8_t unk0; /* 0x03 */
    uint8_t unk1; /* 0x1d */
    uint8_t unk2; /* 0x85 */
    uint8_t seq_no;
    uint8_t status;
    uint8_t unk3; /* 0x00 */
    uint8_t coin_count;
    uint8_t unk4; /* 0xfd */
    union {
        uint32_t inverted_pad;
        struct {
            uint8_t  sys; /* test/service bitfield */
            uint16_t button; /* button bitfield */
            uint8_t  dipsw; /* dip switches */
        };
    } io;
    uint8_t unk5; /* 0x00 */
    uint8_t unk6; /* 0x7d */
    uint8_t unk7; /* 0xdf */
    uint8_t unk8; /* seq no 2? */
    uint16_t button_history[10];
    uint8_t padding[28];
};
#pragma pack(pop)

#endif
