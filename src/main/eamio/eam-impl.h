#ifndef EAMIO_EAM_IMPL_H
#define EAMIO_EAM_IMPL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bemanitools/eamio.h"

#include "geninput/hid-mgr.h"

#define EAM_CARD_NBYTES         8
#define EAM_UNIT_COUNT          2

struct eam;

struct eam *eam_impl_create(void);
bool eam_impl_get_autogen(struct eam *eam);
void eam_impl_set_autogen(struct eam *eam, bool autogen);
bool eam_impl_get_alt_10k(struct eam *eam);
void eam_impl_set_alt_10k(struct eam *eam, bool alt_10k);
struct hid_stub *eam_impl_get_keypad_device(struct eam *eam, uint8_t unit_no);
void eam_impl_set_keypad_device(struct eam *eam, uint8_t unit_no,
        struct hid_stub *hid);
const char *eam_impl_get_card_path(struct eam *eam, uint8_t unit_no);
void eam_impl_set_card_path(struct eam *eam, uint8_t unit_no,
        const char *path);
uint16_t eam_impl_get_keypad_state(struct eam *eam, uint8_t unit_no);
bool eam_impl_get_sensor_state(struct eam *eam, uint8_t unit_no);
uint8_t eam_impl_read_card(struct eam *eam, uint8_t unit_no, uint8_t *card_id,
        uint8_t nbytes);
void eam_impl_notify_hotplug(struct eam *eam, uint8_t drive_no);
void eam_impl_destroy(struct eam *eam);

#endif
