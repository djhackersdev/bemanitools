#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "util/mem.h"

typedef struct input_key_state {
    bool previous_update_state;
    bool current_update_state;
} input_key_state_t;

typedef struct input {
    input_key_state_t esc; 
    input_key_state_t up;
    input_key_state_t down;
    input_key_state_t left;
    input_key_state_t right;
    input_key_state_t enter;
} input_t;

void input_init(input_t **input)
{
    assert(input);

    *input = xmalloc(sizeof(input_t));
    memset(*input, 0, sizeof(input_t));
}

void input_update(input_t *input)
{
    assert(input);

    input->esc.previous_update_state = input->esc.current_update_state;
    input->esc.current_update_state = GetAsyncKeyState(VK_ESCAPE) & 0x8000;

    input->up.previous_update_state = input->up.current_update_state;
    input->up.current_update_state = GetAsyncKeyState(VK_UP) & 0x8000;

    input->down.previous_update_state = input->down.current_update_state;
    input->down.current_update_state = GetAsyncKeyState(VK_DOWN) & 0x8000;

    input->left.previous_update_state = input->left.current_update_state;
    input->left.current_update_state = GetAsyncKeyState(VK_LEFT) & 0x8000;

    input->right.previous_update_state = input->right.current_update_state;
    input->right.current_update_state = GetAsyncKeyState(VK_RIGHT) & 0x8000;

    input->enter.previous_update_state = input->enter.current_update_state;
    input->enter.current_update_state = GetAsyncKeyState(VK_RETURN) & 0x8000;
    
}

bool input_key_esc_pushed(input_t *input)
{
    return input->esc.previous_update_state == false && input->esc.current_update_state == true;
}

bool input_key_up_pushed(input_t *input)
{
    return input->up.previous_update_state == false && input->up.current_update_state == true;
}

bool input_key_down_pushed(input_t *input)
{
    return input->down.previous_update_state == false && input->down.current_update_state == true;
}

bool input_key_left_pushed(input_t *input)
{
    return input->left.previous_update_state == false && input->left.current_update_state == true;
}

bool input_key_right_pushed(input_t *input)
{
    return input->right.previous_update_state == false && input->right.current_update_state == true;
}

bool input_key_enter_pushed(input_t *input)
{
    return input->enter.previous_update_state == false && input->enter.current_update_state == true;
}

void input_fini(input_t *input)
{
    assert(input);

    free(input);
}