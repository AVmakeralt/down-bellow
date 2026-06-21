#ifndef TV_PLAYER_H
#define TV_PLAYER_H

#include <stdbool.h>
#include "types.h"
#include "input.h"
#include "level.h"

/*
 * The Voided (player character).
 *
 * States:
 *   IDLE    on ground, no horizontal input
 *   WALK    on ground, holding L/R
 *   JUMP    in air, vy < 0
 *   FALL    in air, vy >= 0
 *   ATTACK  voidslash active (locks horizontal control briefly)
 *   POGO    downward voidslash active (locks control, applies bounce)
 *   HURT    taking damage (brief i-frames)
 *   SPRINT  mindless sprint: void tentacles, i-frames, env destruction
 *
 * Sprinting is the central verb. See player_sprint.c for the tentacle IK.
 */

typedef enum {
    PS_IDLE = 0,
    PS_WALK,
    PS_JUMP,
    PS_FALL,
    PS_ATTACK,
    PS_POGO,
    PS_HURT,
    PS_SPRINT
} PlayerState;

typedef enum {
    ATTACK_NONE = 0,
    ATTACK_SLASH,
    ATTACK_POGO,
} AttackKind;

/* Live attack hitbox (valid while player.attack_timer > 0) */
typedef struct {
    AttackKind kind;
    Rect box;
    int remaining_ticks;
    bool frozen;          /* mid-parry: hitbox frozen, no damage yet */
    int  freeze_ticks;    /* countdown during a parry freeze */
    int  freeze_total;    /* total freeze ticks at start (for perfect-window calc) */
    bool perfect_used;    /* true if perfect-parry bonus has been applied */
} AttackHitbox;

typedef struct {
    Vec2 pos;            /* top-left of collision box */
    Vec2 vel;
    int  facing;         /* +1 right, -1 left */
    PlayerState state;

    int  anim_timer;     /* incremented every update tick; drives walk anim */

    int  coyote;         /* ticks since last grounded */
    int  jump_buffer;    /* ticks a jump press will still trigger */

    int  attack_timer;   /* ticks remaining in current attack */
    int  attack_cooldown;/* ticks before next attack */
    bool attack_hit;     /* already dealt damage with this swing */
    bool pogo_hit;       /* already bounced off something this pogo */

    int  hurt_timer;     /* i-frame countdown */
    int  hp;

    /* kenotita: void resource. Spent on manual sprint, refilled on
     * perfect parry + slow passive regen. */
    float kenotita;

    /* mindless sprint */
    int  sprint_timer;   /* > 0 = sprinting for this many ticks */
    int  sprint_iframes; /* i-frame countdown at start of sprint */
    bool sprint_panic;   /* true if this sprint was panic-auto (no control) */
    int  panic_lock;     /* ticks of lost control after panic sprint */

    /* hit-stop hook: while > 0, the world doesn't update */
    int  hitstop;
} Player;

void player_init(Player* p, Vec2 spawn);
void player_update(Player* p, AttackHitbox* atk, const Input* in,
                   const Level* lvl);

Rect player_bounds(const Player* p);

/* Begin a manual mindless sprint if kenotita allows. Returns true if started. */
bool player_try_sprint(Player* p, const Input* in);

/* Force-trigger a panic sprint (called by world when overwhelming enemy
 * count is in aggro range). Free, but locks control for PANIC_LOCK_TICKS. */
void player_force_panic_sprint(Player* p);

/* Returns true if the player is currently intangible (sprint i-frames). */
bool player_is_intangible(const Player* p);

#endif /* TV_PLAYER_H */
