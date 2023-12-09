#ifndef GENINPUT_INPUT_CONFIG_H
#define GENINPUT_INPUT_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/input.h"

#include "geninput/mapper.h"

void mapper_config_save(const char *game_type);
void mapper_clear_action_map(uint8_t action, uint8_t page);
void mapper_clear_light_map(const struct mapped_light *ml);
bool mapper_get_action_map(
    uint8_t action, uint8_t page, struct mapped_action *ma);
bool mapper_get_analog_map(uint8_t analog, struct mapped_analog *ma);
int32_t mapper_get_analog_sensitivity(uint8_t analog);
bool mapper_get_analog_invert(uint8_t analog);
uint8_t mapper_get_nanalogs(void);
uint8_t mapper_get_nlights(void);
uint8_t mapper_get_npages(void);
bool mapper_is_analog_absolute(uint8_t analog);
action_iter_t mapper_iterate_actions(void);
light_iter_t mapper_iterate_lights(void);
void mapper_set_action_map(
    uint8_t action, uint8_t page, uint8_t bit, const struct mapped_action *ma);
bool mapper_set_analog_map(uint8_t analog, const struct mapped_analog *ma);
bool mapper_set_analog_sensitivity(uint8_t analog, int32_t sensitivity);
bool mapper_set_analog_invert(uint8_t analog, bool invert);
void mapper_set_nanalogs(uint8_t nanalogs);
void mapper_set_nlights(uint8_t nlights);
void mapper_set_light_map(const struct mapped_light *ml, uint8_t game_light);

#endif
