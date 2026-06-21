#include "combat.h"
#include "config.h"

#include <string.h>

/*
 * Parry resolution rules (from the design doc):
 *
 *   When the player's attack clashes with an enemy's attack, both freeze
 *   for ~20 frames, leaving the hitboxes in place.
 *
 *   - If the player's hitbox is still overlapping the enemy when the
 *     freeze ends: player WINS -> enemy stunned, takes damage.
 *   - If neither hitbox overlaps the other when freeze ends: it's a
 *     clean fizzle -> both attacks end, no damage.
 *   - If only the enemy's hitbox overlaps the player when freeze ends:
 *     enemy WINS -> player takes damage.
 */

static void player_take_hit(Player* p, int dmg) {
    if (p->hurt_timer > 0) return;     /* i-frames */
    p->hp -= dmg;
    p->hurt_timer = 40;
    p->state = PS_HURT;
    /* small knockback */
    p->vel.y = -4.0f;
    p->vel.x = -p->facing * 3.0f;
}

static void enemy_take_hit(Enemy* e, int dmg) {
    e->hp -= dmg;
    if (e->hp <= 0) {
        e->state = ES_DEAD;
        e->attack_active = false;
        return;
    }
    enemy_set_state(e, ES_HURT);
    e->attack_active = false;
    e->vel.x = -e->facing * 2.5f;
}

void combat_resolve(Player* p, AttackHitbox* p_atk, Enemy* enemies, int n) {
    Rect pb = player_bounds(p);

    /* ---- advance existing parry freezes ---- */
    if (p_atk->frozen && p_atk->freeze_ticks > 0) {
        p_atk->freeze_ticks--;
        if (p_atk->freeze_ticks == 0) {
            /* resolve outcome: did player or enemy win? */
            bool player_won = false;
            bool enemy_won  = false;
            for (int i = 0; i < n; i++) {
                Enemy* e = &enemies[i];
                if (e->state == ES_DEAD) continue;
                if (!e->attack_frozen) continue;
                e->freeze_ticks = 0;
                e->attack_frozen = false;

                Rect eb = enemy_bounds(e);
                Rect ab = e->attack_box;
                bool player_hits_enemy  = rects_overlap(p_atk->box, eb);
                bool enemy_hits_player  = rects_overlap(ab, pb);

                if (player_hits_enemy) {
                    player_won = true;
                    /* enemy takes a clean hit + stun */
                    e->hp -= 2;
                    if (e->hp <= 0) {
                        e->state = ES_DEAD;
                    } else {
                        enemy_set_state(e, ES_STUN);
                    }
                    e->attack_active = false;
                } else if (enemy_hits_player) {
                    enemy_won = true;
                } else {
                    /* fizzle: enemy returns to recovery */
                    enemy_set_state(e, ES_RECOVERY);
                    e->attack_active = false;
                }
            }
            if (enemy_won && !player_won) {
                player_take_hit(p, 1);
            }
            p_atk->frozen = false;
            p_atk->kind = ATTACK_NONE;
            p_atk->remaining_ticks = 0;
            p->attack_timer = 0;
            p->state = (p->state == PS_POGO) ? PS_FALL : PS_IDLE;
        }
        /* while frozen, skip normal resolution */
        return;
    }

    /* ---- if player is mid-attack, check for parry clash ---- */
    if (p_atk->kind != ATTACK_NONE && p_atk->remaining_ticks > 0) {
        for (int i = 0; i < n; i++) {
            Enemy* e = &enemies[i];
            if (e->state == ES_DEAD) continue;
            if (!e->attack_active) continue;
            if (!rects_overlap(p_atk->box, e->attack_box)) continue;
            /* CLASH! Freeze both for PARRY_FREEZE_TICKS. */
            p_atk->frozen = true;
            p_atk->freeze_ticks = PARRY_FREEZE_TICKS;
            e->attack_frozen = true;
            e->freeze_ticks = PARRY_FREEZE_TICKS;
            return;
        }
    }

    /* ---- player voidslash hits enemy body (no clash) ---- */
    if (p_atk->kind != ATTACK_NONE && p_atk->remaining_ticks > 0 && !p_atk->frozen) {
        for (int i = 0; i < n; i++) {
            Enemy* e = &enemies[i];
            if (e->state == ES_DEAD) continue;
            if (e->attack_active) continue;  /* would have clashed above */
            Rect eb = enemy_bounds(e);
            if (!rects_overlap(p_atk->box, eb)) continue;
            /* pogo bounce if it's a pogo attack */
            if (p_atk->kind == ATTACK_POGO) {
                if (!p->pogo_hit) {
                    p->vel.y = POGO_VELOCITY;
                    p->pogo_hit = true;
                }
            }
            enemy_take_hit(e, 1);
            p_atk->remaining_ticks = 0;
            p_atk->kind = ATTACK_NONE;
            p->attack_timer = 0;
            p->state = (p_atk->kind == ATTACK_POGO) ? PS_FALL : PS_IDLE;
            break;
        }
    }

    /* ---- enemy attack hits player body ---- */
    for (int i = 0; i < n; i++) {
        Enemy* e = &enemies[i];
        if (e->state == ES_DEAD) continue;
        if (!e->attack_active) continue;
        if (e->attack_hit) continue;
        if (!rects_overlap(e->attack_box, pb)) continue;
        player_take_hit(p, 1);
        e->attack_hit = true;
    }
}
