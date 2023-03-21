#ifndef VIGEM_IIDXIO_CAB_LIGHT_SEQUENCER_H
#define VIGEM_IIDXIO_CAB_LIGHT_SEQUENCER_H

#include <stdbool.h>
#include <stdint.h>

enum vigem_iidxio_cab_light_sequencer_mode {
    LIGHT_SEQ_MODE_OFF = 0,
    LIGHT_SEQ_MODE_NEONS_FLASH = 1,
    LIGHT_SEQ_MODE_NEONS_FLASH_TT_INPUT = 2,
};

void vigem_iidxio_cab_light_sequencer_init(
    enum vigem_iidxio_cab_light_sequencer_mode light_seq_mode);

void vigem_iidxio_cab_light_sequencer_update(
    uint16_t keys,
    uint8_t tt_p1,
    uint8_t tt_p2,
    bool *out_neon,
    uint8_t *out_spots);

#endif