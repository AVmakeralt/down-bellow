#ifndef TV_INPUT_H
#define TV_INPUT_H

#include <stdbool.h>
#include <SDL2/SDL.h>

/*
 * Input layer. We snapshot keyboard + gamepad state into a struct once per
 * tick so gameplay code never touches SDL directly. Also tracks
 * "pressed this tick" / "released this tick" edges for jump/attack/sprint.
 *
 * Gamepad: any SDL_GameController that's open at input_init time is polled
 * each frame. D-pad and left stick map to BTN_LEFT/RIGHT/UP/DOWN. A button
 * = jump, X button = attack, Y = pogo trigger, B = sprint, Start = quit,
 * Back = debug.
 */

typedef enum {
    BTN_LEFT = 0,
    BTN_RIGHT,
    BTN_UP,
    BTN_DOWN,
    BTN_JUMP,
    BTN_ATTACK,
    BTN_POGO,
    BTN_SPRINT,         /* manual mindless sprint trigger */
    BTN_PAUSE,
    BTN_SAVE,           /* F5: manual save (works anywhere) */
    BTN_QUIT,
    BTN_DEBUG,          /* F1 */
    BTN_SLOWMO,         /* F2 */
    BTN_COUNT
} Button;

typedef struct {
    bool down[BTN_COUNT];        /* currently held                      */
    bool pressed[BTN_COUNT];     /* went down this tick                 */
    bool released[BTN_COUNT];    /* went up this tick                   */

    SDL_GameController* pad;     /* first opened gamepad, or NULL       */
} Input;

void input_init(Input* in);
void input_shutdown(Input* in);
void input_begin_frame(Input* in);
void input_handle_event(Input* in, const SDL_Event* e);
void input_end_frame(Input* in);

/* Pump gamepad state into the button struct (called once per tick). */
void input_poll_gamepad(Input* in);

#endif /* TV_INPUT_H */
