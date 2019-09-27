#ifndef LIBMM_MM_H
#define LIBMM_MM_H

#include <stdatomic.h>
#include <stdbool.h>

#define MM_VENDOR_ID        0xBEEF
#define MM_PRODUCT_ID       0x5730

#pragma pack(push, 1)

struct mm_input {
    uint8_t report_id;
    uint8_t dip_switches;
    uint32_t jamma;
    uint8_t ext_in;
};

struct mm_output {
    uint8_t report_id;
    uint8_t ext_output;
    _Atomic uint32_t lights;
    uint8_t blue_led;
    uint8_t kbd_enable;
    uint8_t aux_flags;
};

#pragma pack(pop)

bool mm_init(uint32_t in_buf_count);
void mm_update(const struct mm_output *out, struct mm_input *in);
void mm_fini(void);

#endif
