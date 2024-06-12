#ifndef BT_API_INPUT_H
#define BT_API_INPUT_H

#include <stdbool.h>
#include <stdint.h>

typedef bool (*bt_input_init_t)();
typedef void (*bt_input_fini_t)();
typedef bool (*bt_input_mapper_config_load_t)(const char *game_type);
typedef uint8_t (*bt_input_mapper_analog_read_t)(uint8_t analog);
typedef uint64_t (*bt_input_mapper_update_t)();
typedef void (*bt_input_mapper_light_write_t)(uint8_t light, uint8_t intensity);

typedef struct bt_input_api {
    uint16_t version;

    struct {
        // Required to be implemented
         bt_input_init_t init;
         bt_input_fini_t fini;
         bt_input_mapper_config_load_t mapper_config_load;
         bt_input_mapper_analog_read_t mapper_analog_read;
         bt_input_mapper_update_t mapper_update;
         bt_input_mapper_light_write_t mapper_light_write;
    } v1;
} bt_input_api_t;

#endif
