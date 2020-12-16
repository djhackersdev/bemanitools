#ifndef VIGEM_IIDXIO_CAB_LIGHT_SEQUENCER_H
#define VIGEM_IIDXIO_CAB_LIGHT_SEQUENCER_H

#include <stdbool.h>
#include <stdint.h>

void vigem_iidxio_cab_light_sequencer_init();

void vigem_iidxio_cab_light_sequencer_update(bool* out_neon, uint8_t* out_spots);

#endif