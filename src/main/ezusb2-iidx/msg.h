#ifndef EZUSB2_IIDX_MSG_H
#define EZUSB2_IIDX_MSG_H

#include <stdint.h>
#include <stddef.h>

#pragma pack(push, 1)
struct ezusb2_iidx_msg_interrupt_write_packet {
    uint8_t unk0;
    uint8_t unk1;
    uint8_t node;
    uint8_t cmd;
    uint8_t cmd_detail[2];
    uint8_t unk2;
    uint8_t unk3;
    uint8_t panel_lights;
    uint8_t unk4;
    uint8_t unk5;
    uint16_t deck_lights;
    uint8_t unk6;
    uint8_t top_lamps;
    uint8_t top_neons;
    uint8_t seg16[9];
    /* Pad to 64 byte total size because the endpoint expects this buffer size.
       Requests are hanging on ioctl calls otherwise. */
    uint8_t padding[39];
};
#pragma pack(pop)

/* Some static asserts to ensure that lights are at correct offsets */
_Static_assert(offsetof(struct ezusb2_iidx_msg_interrupt_write_packet, 
    panel_lights) == 8, "panel_lights is at wrong offset for fx2 packet");
_Static_assert(offsetof(struct ezusb2_iidx_msg_interrupt_write_packet, 
    deck_lights) == 11, "deck_lights is at wrong offset for fx2 packet");
_Static_assert(offsetof(struct ezusb2_iidx_msg_interrupt_write_packet, 
    top_lamps) == 14, "top_lamps is at wrong offset for fx2 packet");
_Static_assert(offsetof(struct ezusb2_iidx_msg_interrupt_write_packet, 
    top_neons) == 15, "top_neons is at wrong offset for fx2 packet");
_Static_assert(offsetof(struct ezusb2_iidx_msg_interrupt_write_packet, 
    padding[39]) == 64, "Last padding element is at wrong offset for fx2 write packet");

struct ezusb2_iidx_msg_interrupt_read_packet {
    /*
      Pad mappings:
      0: P1_Start
	  1: P2_Start
	  2: VEFX
	  3: Effector
	  4: Test
	  5: Service
	  6: unknown/not used
	  7: unknown/not used

      8: unknown/not used
	  9: unknown/not used
	  10: unknown/not used
	  11: unknown/not used
	  12: unknown/not used
	  13: unknown/not used
	  14: unknown/not used
	  15: unknown/not used

	  16: P1_1
	  17: P1_2
	  18: P1_3
	  19: P1_4
	  20: P1_5
	  21: P1_6
	  22: P1_7
	  23: P2_1

	  24: P2_2
	  25: P2_3
	  26: P2_4
	  27: P2_5
	  28: P2_6
	  29: P2_7
	  30: Coin mech
	  31: Not used
    */
    uint8_t unk0;
    uint8_t unk1;
    uint8_t unk2;
    uint8_t seq_no;
    uint8_t status;
    uint8_t unk3;
    uint8_t unk4;
    uint8_t unk5;
    uint32_t inverted_pad;
    uint8_t unk6;
    uint8_t p2_turntable;
    uint8_t p1_turntable;
    uint8_t sliders[3];
    /* Pad to 64 byte total size because the endpoint expects this buffer size.
       Requests are hanging on ioctl calls otherwise. */
    uint8_t padding[46];
};

_Static_assert(offsetof(struct ezusb2_iidx_msg_interrupt_read_packet, 
    padding[45]) == 63, "Last padding element is at wrong offset for fx2 read packet");

#endif
