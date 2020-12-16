#ifndef VIGEM_IIDXIO_CAB_16SEG_SEQUENCER_H
#define VIGEM_IIDXIO_CAB_16SEG_SEQUENCER_H

void vigem_iidxio_cab_16seg_sequencer_init(const char* text, uint32_t scroll_cycle_time_ms);

void vigem_iidxio_cab_16seg_sequencer_update(char* out_16seg);

#endif