#ifndef GENINPUT_MAPPER_H
#define GENINPUT_MAPPER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "geninput/hid-mgr.h"

#include "util/defs.h"

typedef struct action_iter *action_iter_t;
typedef struct light_iter *light_iter_t;

struct mapper;

struct mapped_action {
    struct hid_stub *hid;
    uint32_t control_no;
    int32_t value_min;
    int32_t value_max;
};

struct mapped_analog {
    struct hid_stub *hid;
    size_t control_no;
};

struct mapped_light {
    struct hid_stub *hid;
    size_t light_no;
};

extern struct mapper *mapper_inst;

void action_iter_get_mapping(action_iter_t iter, struct mapped_action *ma);
uint8_t action_iter_get_action(action_iter_t iter);
uint8_t action_iter_get_page(action_iter_t iter);
uint8_t action_iter_get_bit(action_iter_t iter);
bool action_iter_is_valid(action_iter_t iter);
void action_iter_next(action_iter_t iter);
void action_iter_free(action_iter_t iter);

void light_iter_get_mapping(light_iter_t iter, struct mapped_light *ml);
uint8_t light_iter_get_game_light(light_iter_t iter);
bool light_iter_is_valid(light_iter_t iter);
void light_iter_next(light_iter_t iter);
void light_iter_free(light_iter_t iter);

struct mapper *mapper_impl_create(void);
void mapper_impl_clear_action_map(struct mapper *m, uint8_t action,
        uint8_t page);
void mapper_impl_clear_light_map(struct mapper *m,
        const struct mapped_light *ml);
bool mapper_impl_get_analog_map(struct mapper *m, uint8_t analog,
        struct mapped_analog *ma);
bool mapper_impl_get_action_map(struct mapper *m, uint8_t action, uint8_t page,
        struct mapped_action *ma);
int32_t mapper_impl_get_analog_sensitivity(struct mapper *m, uint8_t analog);
uint8_t mapper_impl_get_nanalogs(struct mapper *m);
uint8_t mapper_impl_get_nlights(struct mapper *m);
uint8_t mapper_impl_get_npages(struct mapper *m);
bool mapper_impl_is_analog_absolute(struct mapper *m, uint8_t analog);
action_iter_t mapper_impl_iterate_actions(struct mapper *m);
light_iter_t mapper_impl_iterate_lights(struct mapper *m);
void mapper_impl_set_action_map(struct mapper *m, uint8_t action, uint8_t page,
        uint8_t bit, const struct mapped_action *ma);
bool mapper_impl_set_analog_map(struct mapper *m, uint8_t analog,
        const struct mapped_analog *ma);
bool mapper_impl_set_analog_sensitivity(struct mapper *m, uint8_t analog,
        int32_t sensitivity);
void mapper_impl_set_light_map(struct mapper *m, const struct mapped_light *ml,
        uint8_t game_light);
void mapper_impl_set_nanalogs(struct mapper *m, uint8_t nanalogs);
void mapper_impl_set_nlights(struct mapper *m, uint8_t nlights);
uint8_t mapper_impl_read_analog(struct mapper *m, uint8_t analog);
uint64_t mapper_impl_update(struct mapper *m);
void mapper_impl_write_light(struct mapper *m, uint8_t light,
        uint8_t intensity);
void mapper_impl_destroy(struct mapper *m);

#endif
