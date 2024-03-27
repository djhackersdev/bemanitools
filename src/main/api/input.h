#ifndef API_INPUT_H
#define API_INPUT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct api_input api_input_t;

void api_input_load(const char *path, api_input_t **input);
void api_input_free(api_input_t **input);

bool api_input_init(const api_input_t *input);
void api_input_fini(const api_input_t *input);
bool api_input_mapper_config_load(const api_input_t *input, const char *game_type);
uint8_t api_input_mapper_analog_read(const api_input_t *input, uint8_t analog);
uint64_t api_input_mapper_update(const api_input_t *input);
void api_input_mapper_light_write(const api_input_t *input, uint8_t light, uint8_t intensity);

#endif