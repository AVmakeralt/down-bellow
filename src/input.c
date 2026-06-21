#include "input.h"

#include <string.h>

static SDL_Scancode keymap[BTN_COUNT] = {
    [BTN_LEFT]   = SDL_SCANCODE_LEFT,
    [BTN_RIGHT]  = SDL_SCANCODE_RIGHT,
    [BTN_UP]     = SDL_SCANCODE_UP,
    [BTN_DOWN]   = SDL_SCANCODE_DOWN,
    [BTN_JUMP]   = SDL_SCANCODE_Z,        /* Z = jump (Hollow Knight-ish)   */
    [BTN_ATTACK] = SDL_SCANCODE_X,        /* X = voidslash                  */
    [BTN_POGO]   = SDL_SCANCODE_C,        /* C = alt pogo trigger (also down+attack) */
    [BTN_QUIT]   = SDL_SCANCODE_ESCAPE,
    [BTN_DEBUG]  = SDL_SCANCODE_F1,
    [BTN_SLOWMO] = SDL_SCANCODE_F2,
};

void input_init(Input* in) {
    memset(in, 0, sizeof(*in));
}

void input_begin_frame(Input* in) {
    /* Edge flags persist exactly one tick. Clear them so each tick starts
     * clean. 'down' state is preserved across the clear. */
    for (int i = 0; i < BTN_COUNT; i++) {
        in->pressed[i]  = false;
        in->released[i] = false;
    }
}

static void set_button(Input* in, Button b, bool is_down) {
    if (b < 0 || b >= BTN_COUNT) return;
    bool was_down = in->down[b];
    in->down[b] = is_down;
    if (is_down && !was_down)  in->pressed[b]  = true;
    if (!is_down && was_down)  in->released[b] = true;
}

void input_handle_event(Input* in, const SDL_Event* e) {
    switch (e->type) {
        case SDL_QUIT:
            set_button(in, BTN_QUIT, true);
            break;
        case SDL_KEYDOWN: {
            if (e->key.repeat) break;
            const SDL_Scancode sc = e->key.keysym.scancode;
            for (int b = 0; b < BTN_COUNT; b++) {
                if (keymap[b] == sc) {
                    set_button(in, (Button)b, true);
                    break;
                }
            }
            break;
        }
        case SDL_KEYUP: {
            const SDL_Scancode sc = e->key.keysym.scancode;
            for (int b = 0; b < BTN_COUNT; b++) {
                if (keymap[b] == sc) {
                    set_button(in, (Button)b, false);
                    break;
                }
            }
            break;
        }
        default:
            break;
    }
}

void input_end_frame(Input* in) {
    (void)in;
}
