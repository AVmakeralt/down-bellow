#ifndef TV_PLAYER_H
#define TV_PLAYER_H

#include <stdbool.h>
#include "types.h"
#include "input.h"
#include "level.h"

/*
 * The Voided (player character).
 *
 * State machine (simple):
 *   IDLE    -> on ground, no horizontal input
 *   WALK    -> on ground, holding L/R
 *   JUMP    -> in air, vy < 0
 *   FALL    -> in air, vy >= 0
 *   ATTACK  -> voidslash active (locks horizontal control briefly)
 *   POGO    -> downward voidslash active (locks control, applies bounce)
 *   HURT    -> taking damage (brief i-frames)
 */

typedef enum {
    PS_IDLE = 0,
    PS_WALK,
    PS_JUMP,
    PS_FALL,
    PS_ATTACK,
    PS_POGO,
    PS_HURT
} PlayerState;

typedef struct {
    Vec2 pos;            /* top-left of collision box */
    Vec2 vel;
    int  facing;         /* +1 right, -1 left */
    PlayerState state;

    int  coyote;         /* ticks since last grounded */
    int  jump_buffer;    /* ticks a jump press will still trigger */

    int  attack_timer;   /* ticks remaining in current attack */
    int  attack_cooldown;/* ticks before next attack */
    bool attack_hit;     /* already dealt damage with this swing */
    bool pogo_hit;       /* already bounced off something this pogo */

    int  hurt_timer;     /* i-frame countdown */
    int  hp;
} Player;

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
    bool resolved;        /* parry outcome decided */
} AttackHitbox;

void player_init(Player* p, Vec2 spawn);
void player_update(Player* p, AttackHitbox* atk, const Input* in,
                   const Level* lvl);

/* Returns the player's collision AABB */
Rect player_bounds(const Player* p);

#endif /* TV_PLAYER_H */
