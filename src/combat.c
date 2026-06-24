#include "combat.h"
#include "config.h"
#include "tunables.h"
#include "world.h"
#include "camera.h"
#include "audio.h"
#include "particles.h"

#include <string.h>

/*
 * Combat resolution. See combat.h for the overview.
 *
 * Parry flow:
 *   1. Player attack hitbox overlaps enemy attack hitbox  -> CLASH
 *      Both hitboxes freeze for PARRY_FREEZE_TICKS.
 *      The first PARRY_PERFECT_WINDOW ticks of the freeze = perfect window.
 *      If the player pressed attack within that window, mark the parry
 *      as "perfect" for bonus damage + kenotita refund.
 *   2. When freeze ends, resolve:
 *      - Player's hitbox still overlaps enemy body -> player WINS.
 *        Enemy takes damage (2 normal, 4 perfect) + STUN.
 *      - Only enemy's hitbox overlaps player body -> enemy WINS.
 *        Player takes damage (unless i-frames).
 *      - Otherwise -> fizzle, both attacks end.
 */

static void player_take_hit(Player* p, int dmg, World* w) {
    if (p->hurt_timer > 0) return;
    if (player_is_intangible(p)) return;   /* sprint i-frames */
    if (p->dead) return;
    p->hp -= dmg;
    p->hurt_timer = 40;
    p->vel.y = -4.0f;
    p->vel.x = -p->facing * 3.0f;
    p->hitstop = g_tunables.hitstop_hit;
    if (p->hp <= 0) {
        p->hp = 0;
        p->dead = true;
        p->death_timer = 90;   /* 1.5s death animation before respawn */
        p->hitstop = g_tunables.hitstop_kill;
        if (w) {
            camera_add_trauma(&w->camera, SHAKE_TRAUMA_KILL);
            Vec2 pc = rect_center(player_bounds(p));
            particles_burst(&w->particles, pc, 24, PT_VOID, vec2(0, -0.3f), 5.0f);
            particles_burst(&w->particles, pc, 16, PT_BLOOD, vec2(0, -0.8f), 6.0f);
            if (w->audio) audio_play(w->audio, SFX_KILL);
        }
        return;
    }
    p->state = PS_HURT;
    if (w) {
        w->effects.enemy_hit_player = true;
        w->effects.last_event_pos = rect_center(player_bounds(p));
    }
}

static void enemy_take_hit(Enemy* e, int dmg, World* w, Vec2 at_pos) {
    if (e->vtable && e->vtable->on_hit) {
        e->vtable->on_hit(e, dmg);
    }
    if (e->state == ES_DEAD) {
        if (w) {
            w->effects.player_killed_enemy = true;
            w->effects.last_event_pos = at_pos;
            w->effects.last_event_severity = 4;
        }
    }
    if (w) {
        w->effects.player_hit_enemy = true;
        w->effects.last_event_pos = at_pos;
        if (w->effects.last_event_severity < 1) w->effects.last_event_severity = 1;
    }
}

void combat_resolve(Player* p, AttackHitbox* p_atk, Enemy* enemies, int n,
                    World* w) {
    Rect pb = player_bounds(p);

    /* ---- advance existing parry freezes ---- */
    if (p_atk->frozen && p_atk->freeze_ticks > 0) {
        p_atk->freeze_ticks--;
        /* perfect window check: if the player presses attack during the
         * first PARRY_PERFECT_WINDOW ticks of the freeze, mark perfect. */
        if (!p_atk->perfect_used) {
            int elapsed = p_atk->freeze_total - p_atk->freeze_ticks;
            if (elapsed <= g_tunables.parry_perfect_window) {
                /* auto-grant perfect if the player's input frame was good
                 * (in this prototype we treat any sustained clash in the
                 * first 4 ticks as perfect, since the clash itself is the
                 * player's input). */
                p_atk->perfect_used = true;
                if (w) {
                    w->effects.perfect_parry = true;
                    w->effects.last_event_pos = rect_center(p_atk->box);
                    w->effects.last_event_severity = 3;
                }
            }
        }
        if (p_atk->freeze_ticks == 0) {
            /* resolve outcome */
            bool player_won = false;
            bool enemy_won  = false;
            for (int i = 0; i < n; i++) {
                Enemy* e = &enemies[i];
                if (e->state == ES_DEAD) continue;
                if (!e->attack_frozen) continue;
                e->freeze_ticks = 0;
                e->attack_frozen = false;

                Rect eb = e->vtable->hurtbox(e);
                Rect ab = e->attack_box;
                bool player_hits_enemy = rects_overlap(p_atk->box, eb);
                bool enemy_hits_player = rects_overlap(ab, pb);

                if (player_hits_enemy) {
                    player_won = true;
                    int dmg = p_atk->perfect_used ? 4 : 2;
                    if (e->vtable && e->vtable->on_parried) {
                        /* on_parried applies damage + stun internally */
                        e->vtable->on_parried(e, p);
                        /* but if perfect, apply bonus damage */
                        if (p_atk->perfect_used && e->state != ES_DEAD) {
                            e->vtable->on_hit(e, 2);
                        }
                    }
                    (void)dmg;
                    e->attack_active = false;
                } else if (enemy_hits_player) {
                    enemy_won = true;
                } else {
                    /* fizzle */
                    if (e->vtable) enemy_set_state(e, ES_RECOVERY);
                    e->attack_active = false;
                }
            }
            if (enemy_won && !player_won) {
                player_take_hit(p, 1, w);
            }
            /* hit-stop on parry resolution */
            p->hitstop = p_atk->perfect_used
                ? g_tunables.hitstop_perfect
                : g_tunables.hitstop_parry;
            if (w) {
                w->effects.parry_resolved = true;
                w->effects.last_event_pos = rect_center(p_atk->box);
            }
            p_atk->frozen = false;
            p_atk->kind = ATTACK_NONE;
            p_atk->remaining_ticks = 0;
            p->attack_timer = 0;
            p->state = PS_IDLE;   /* combat.c no longer knows pogo vs slash
                                   * after a parry; IDLE is safe */
        }
        return;   /* still frozen, skip normal resolution */
    }

    /* ---- if player is mid-attack, check for parry clash ---- */
    if (p_atk->kind != ATTACK_NONE && p_atk->remaining_ticks > 0) {
        for (int i = 0; i < n; i++) {
            Enemy* e = &enemies[i];
            if (e->state == ES_DEAD) continue;
            if (!e->attack_active) continue;
            Rect ab = e->vtable->attack_hitbox(e);
            if (rects_overlap(p_atk->box, ab)) {
                /* CLASH */
                p_atk->frozen = true;
                p_atk->freeze_ticks = g_tunables.parry_freeze_ticks;
                p_atk->freeze_total = g_tunables.parry_freeze_ticks;
                p_atk->perfect_used = false;
                e->attack_frozen = true;
                e->freeze_ticks = g_tunables.parry_freeze_ticks;
                if (w) {
                    w->effects.parry_started = true;
                    w->effects.last_event_pos = rect_center(p_atk->box);
                    w->effects.last_event_severity = 2;
                }
                return;
            }
        }
    }

    /* ---- player voidslash hits enemy body (no clash) ---- */
    if (p_atk->kind != ATTACK_NONE && p_atk->remaining_ticks > 0 && !p_atk->frozen) {
        for (int i = 0; i < n; i++) {
            Enemy* e = &enemies[i];
            if (e->state == ES_DEAD) continue;
            if (e->attack_active) continue;
            Rect eb = e->vtable->hurtbox(e);
            if (!rects_overlap(p_atk->box, eb)) continue;

            Vec2 hit_pos = rect_center(eb);
            AttackKind dealt_kind = p_atk->kind;

            /* pogo bounce */
            if (dealt_kind == ATTACK_POGO && !p->pogo_hit) {
                p->vel.y = g_tunables.pogo_velocity;
                p->pogo_hit = true;
                if (w) {
                    w->effects.pogo_bounced = true;
                    w->effects.last_event_pos = hit_pos;
                }
            }
            /* tentacle yank during sprint */
            if (p->state == PS_SPRINT) {
                /* yank enemy toward player */
                float yank_dir = (p->pos.x < e->pos.x) ? -1.0f : 1.0f;
                e->pos.x += yank_dir * 6.0f;
            }

            enemy_take_hit(e, 1, w, hit_pos);
            p->hitstop = g_tunables.hitstop_hit;
            p_atk->remaining_ticks = 0;
            p_atk->kind = ATTACK_NONE;
            p->attack_timer = 0;
            p->state = (dealt_kind == ATTACK_POGO) ? PS_FALL : PS_IDLE;
            break;
        }
    }

    /* ---- enemy attack hits player body ---- */
    for (int i = 0; i < n; i++) {
        Enemy* e = &enemies[i];
        if (e->state == ES_DEAD) continue;
        if (!e->attack_active) continue;
        if (e->attack_hit) continue;
        Rect ab = e->vtable->attack_hitbox(e);
        if (!rects_overlap(ab, pb)) continue;
        player_take_hit(p, 1, w);
        e->attack_hit = true;
    }

    /* ---- sprint collision: tentacles damage enemies on contact ---- */
    if (p->state == PS_SPRINT && !player_is_intangible(p)) {
        Rect pb2 = player_bounds(p);
        /* expand by 6px each side to represent tentacle reach */
        Rect tentacle_reach = rect(pb2.x - 6, pb2.y - 6, pb2.w + 12, pb2.h + 12);
        for (int i = 0; i < n; i++) {
            Enemy* e = &enemies[i];
            if (e->state == ES_DEAD) continue;
            Rect eb = e->vtable->hurtbox(e);
            if (!rects_overlap(tentacle_reach, eb)) continue;
            /* one damage per ~10 ticks of contact (use anim_timer mod) */
            if ((p->anim_timer % 10) == 0) {
                Vec2 hit_pos = rect_center(eb);
                enemy_take_hit(e, 1, w, hit_pos);
                p->hitstop = 1;   /* micro hit-stop for sprint impact */
            }
        }
    }
}
