#ifndef TV_INPUT_H
#define TV_INPUT_H

#include <stdbool.h>
#include <SDL2/SDL.h>

/*
 * Input layer. We snapshot keyboard state into a struct once per tick so
 * gameplay code never touches SDL directly. Also tracks "pressed this
 * tick" edges for jump/attack.
 */

typedef enum {
    BTN_LEFT = 0,
    BTN_RIGHT,
    BTN_UP,
    BTN_DOWN,
    BTN_JUMP,
    BTN_ATTACK,
    BTN_POGO,        /* attack while holding down -> pogo */
    BTN_QUIT,
    BTN_DEBUG,       /* F1 */
    BTN_SLOWMO,      /* F2 */
    BTN_COUNT
} Button;

typedef struct {
    bool down[BTN_COUNT];        /* currently held                      */
    bool pressed[BTN_COUNT];     /* went down this tick                 */
    bool released[BTN_COUNT];    /* went up this tick                   */
} Input;

void input_init(Input* in);
void input_begin_frame(Input* in);  /* clears pressed/released edges */
void input_handle_event(Input* in, const SDL_Event* e);
void input_end_frame(Input* in);    /* noop for now, reserved */

#endif /* TV_INPUT_H */
