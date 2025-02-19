#ifndef D3D9_MONITOR_CHECK_INPUT_H
#define D3D9_MONITOR_CHECK_INPUT_H

#include <stdbool.h>

typedef struct input input_t;

void input_init(input_t **input);

void input_update(input_t *input);

bool input_key_esc_pushed(input_t *input);

bool input_key_up_pushed(input_t *input);

bool input_key_down_pushed(input_t *input);

bool input_key_left_pushed(input_t *input);

bool input_key_right_pushed(input_t *input);

bool input_key_enter_pushed(input_t *input);

void input_fini(input_t *input);

#endif
