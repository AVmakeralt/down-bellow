#ifndef TV_ENEMY_H
#define TV_ENEMY_H

#include <stdbool.h>
#include "types.h"
#include "level.h"
#include "player.h"

/*
 * Enemy: Void Crawler
 * -------------------
 * A simple shade that patrols and lunges when the player gets close.
 *
 * State machine:
 *   IDLE     -> standing, scanning for the player
 *   WALK     -> patrolling toward the player (slow)
 *   WINDUP   -> telegraph: claws raised, glows bright (parryable soon)
 *   ATTACK   -> claws forward; this is the parryable hitbox window
 *   RECOVERY -> cooldown after the lunge
 *   STUN     -> lost a parry; player gets a free hit
 *   HURT     -> brief knockback flash
 *   DEAD     -> removed by the world
 */

typedef enum {
    ES_IDLE = 0,
    ES_WALK,
    ES_WINDUP,
    ES_ATTACK,
    ES_RECOVERY,
    ES_STUN,
    ES_HURT,
    ES_DEAD
} EnemyState;

typedef struct {
    Vec2 pos;
    Vec2 vel;
    int  facing;
    EnemyState state;

    int  state_timer;     /* ticks in current state */
    int  hp;
    int  walk_anim;       /* 0/1 for leg swap */

    /* Attack hitbox: valid only while state == ES_ATTACK and the active
     * window is open. Parryable for the whole active window. */
    Rect attack_box;
    bool attack_active;
    bool attack_frozen;   /* mid-parry: hitbox frozen, no damage yet */
    int  freeze_ticks;
    bool attack_hit;      /* already damaged player with this swing */
} Enemy;

void enemy_init(Enemy* e, Vec2 spawn);
void enemy_update(Enemy* e, Player* p, const Level* lvl);
Rect enemy_bounds(const Enemy* e);

/* Transition enemy into a new state, resetting its state_timer. Used by
 * combat.c when resolving parry outcomes. */
void enemy_set_state(Enemy* e, EnemyState s);

#endif /* TV_ENEMY_H */
