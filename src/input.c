#include "input.h"

#include <string.h>

static SDL_Scancode keymap[BTN_COUNT] = {
    [BTN_LEFT]   = SDL_SCANCODE_LEFT,
    [BTN_RIGHT]  = SDL_SCANCODE_RIGHT,
    [BTN_UP]     = SDL_SCANCODE_UP,
    [BTN_DOWN]   = SDL_SCANCODE_DOWN,
    [BTN_JUMP]   = SDL_SCANCODE_Z,        /* Z = jump                    */
    [BTN_ATTACK] = SDL_SCANCODE_X,        /* X = voidslash               */
    [BTN_POGO]   = SDL_SCANCODE_C,        /* C = alt pogo trigger        */
    [BTN_SPRINT] = SDL_SCANCODE_LSHIFT,   /* Shift = mindless sprint     */
    [BTN_PAUSE]  = SDL_SCANCODE_P,
    [BTN_SAVE]    = SDL_SCANCODE_F5,
    [BTN_QUIT]   = SDL_SCANCODE_ESCAPE,
    [BTN_DEBUG]  = SDL_SCANCODE_F1,
    [BTN_SLOWMO] = SDL_SCANCODE_F2,
};

void input_init(Input* in) {
    memset(in, 0, sizeof(*in));
    /* try to open the first available gamepad */
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            in->pad = SDL_GameControllerOpen(i);
            if (in->pad) break;
        }
    }
}

void input_shutdown(Input* in) {
    if (in->pad) {
        SDL_GameControllerClose(in->pad);
        in->pad = NULL;
    }
}

void input_begin_frame(Input* in) {
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
        case SDL_CONTROLLERDEVICEADDED:
            if (!in->pad) {
                in->pad = SDL_GameControllerOpen(e->cdevice.which);
            }
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (in->pad && SDL_GameControllerGetPlayerIndex(in->pad) == e->cdevice.which) {
                SDL_GameControllerClose(in->pad);
                in->pad = NULL;
            }
            break;
        default:
            break;
    }
}

/* deadzone for the left stick */
#define STICK_DEADZONE 8000

void input_poll_gamepad(Input* in) {
    if (!in->pad) return;

    /* D-pad */
    if (SDL_GameControllerGetButton(in->pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT))   set_button(in, BTN_LEFT, true);
    else if (!SDL_GameControllerGetButton(in->pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
        /* fall through to stick; only release if stick also doesn't say left */
    }
    /* For simplicity, we OR the pad buttons with the current keyboard state.
     * We don't release keyboard-set buttons here. The simplest approach for
     * a prototype: read the stick as an alternative source and treat any
     * non-zero stick axis as held, zero as released (overriding keyboard
     * only if the stick is engaged). */

    Sint16 sx = SDL_GameControllerGetAxis(in->pad, SDL_CONTROLLER_AXIS_LEFTX);
    Sint16 sy = SDL_GameControllerGetAxis(in->pad, SDL_CONTROLLER_AXIS_LEFTY);

    if (sx < -STICK_DEADZONE) set_button(in, BTN_LEFT,  true);
    if (sx >  STICK_DEADZONE) set_button(in, BTN_RIGHT, true);
    if (sy < -STICK_DEADZONE) set_button(in, BTN_UP,    true);
    if (sy >  STICK_DEADZONE) set_button(in, BTN_DOWN,  true);

    /* face buttons (assuming Nintendo-style layout swap is handled by SDL) */
    if (SDL_GameControllerGetButton(in->pad, SDL_CONTROLLER_BUTTON_A)) set_button(in, BTN_JUMP,   true);
    if (SDL_GameControllerGetButton(in->pad, SDL_CONTROLLER_BUTTON_X)) set_button(in, BTN_ATTACK, true);
    if (SDL_GameControllerGetButton(in->pad, SDL_CONTROLLER_BUTTON_Y)) set_button(in, BTN_POGO,   true);
    if (SDL_GameControllerGetButton(in->pad, SDL_CONTROLLER_BUTTON_B)) set_button(in, BTN_SPRINT, true);
    if (SDL_GameControllerGetButton(in->pad, SDL_CONTROLLER_BUTTON_START)) set_button(in, BTN_PAUSE, true);
    if (SDL_GameControllerGetButton(in->pad, SDL_CONTROLLER_BUTTON_BACK))  set_button(in, BTN_DEBUG, true);
    /* right shoulder = sprint alt */
    if (SDL_GameControllerGetButton(in->pad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) set_button(in, BTN_SPRINT, true);
}

void input_end_frame(Input* in) {
    (void)in;
}
