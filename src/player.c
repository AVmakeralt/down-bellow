#include "player.h"
#include "config.h"
#include "tunables.h"
#include "level.h"

#include <math.h>
#include <string.h>

void player_init(Player* p, Vec2 spawn) {
    memset(p, 0, sizeof(*p));
    p->pos = spawn;
    p->facing = 1;
    p->state = PS_IDLE;
    p->hp = 5;
    p->kenotita = KENOTITA_MAX;
}

Rect player_bounds(const Player* p) {
    return rect(p->pos.x, p->pos.y, (float)PLAYER_W, (float)PLAYER_H);
}

/* ---- collision helpers (axis-separated) ------------------------- */
static void move_x(Player* p, const Level* lvl, float dx) {
    p->pos.x += dx;
    Rect b = player_bounds(p);
    int tx0 = (int)(b.x / TILE_SIZE);
    int tx1 = (int)((b.x + b.w - 1) / TILE_SIZE);
    int ty0 = (int)(b.y / TILE_SIZE);
    int ty1 = (int)((b.y + b.h - 1) / TILE_SIZE);
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (!level_is_solid(lvl, tx, ty)) continue;
            Rect tile = rect(tx * TILE_SIZE, ty * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            if (!rects_overlap(b, tile)) continue;
            if (dx > 0) {
                p->pos.x = tile.x - PLAYER_W;
                p->vel.x = 0;
            } else if (dx < 0) {
                p->pos.x = tile.x + TILE_SIZE;
                p->vel.x = 0;
            }
            b = player_bounds(p);
        }
    }
}

static bool move_y(Player* p, const Level* lvl, float dy) {
    bool hit = false;
    p->pos.y += dy;
    Rect b = player_bounds(p);
    int tx0 = (int)(b.x / TILE_SIZE);
    int tx1 = (int)((b.x + b.w - 1) / TILE_SIZE);
    int ty0 = (int)(b.y / TILE_SIZE);
    int ty1 = (int)((b.y + b.h - 1) / TILE_SIZE);
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (!level_is_solid(lvl, tx, ty)) continue;
            Rect tile = rect(tx * TILE_SIZE, ty * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            if (!rects_overlap(b, tile)) continue;
            if (dy > 0) {
                p->pos.y = tile.y - PLAYER_H;
                p->vel.y = 0;
                hit = true;
            } else if (dy < 0) {
                p->pos.y = tile.y + TILE_SIZE;
                p->vel.y = 0;
                hit = true;
            }
            b = player_bounds(p);
        }
    }
    return hit;
}

static bool on_ground(const Player* p, const Level* lvl) {
    Rect probe = rect(p->pos.x + 1, p->pos.y + PLAYER_H, PLAYER_W - 2, 1);
    int tx0 = (int)(probe.x / TILE_SIZE);
    int tx1 = (int)((probe.x + probe.w - 1) / TILE_SIZE);
    int ty  = (int)((probe.y + probe.h - 1) / TILE_SIZE);
    for (int x = tx0; x <= tx1; x++) {
        if (level_is_solid(lvl, x, ty)) return true;
    }
    return false;
}

bool player_try_sprint(Player* p, const Input* in) {
    (void)in;
    if (p->kenotita < KENOTITA_SPRINT_COST) return false;
    if (p->sprint_timer > 0) return false;
    if (p->state == PS_HURT) return false;
    p->kenotita -= KENOTITA_SPRINT_COST;
    p->sprint_timer = g_tunables.sprint_duration_ticks;
    p->sprint_iframes = SPRINT_IFRAME_TICKS;
    p->sprint_panic = false;
    p->panic_lock = 0;
    p->state = PS_SPRINT;
    return true;
}

void player_force_panic_sprint(Player* p) {
    if (p->sprint_timer > 0) return;
    if (p->state == PS_HURT) return;
    p->sprint_timer = g_tunables.sprint_duration_ticks;
    p->sprint_iframes = SPRINT_IFRAME_TICKS;
    p->sprint_panic = true;
    p->panic_lock = PANIC_LOCK_TICKS;
    p->state = PS_SPRINT;
}

bool player_is_intangible(const Player* p) {
    return p->sprint_iframes > 0;
}

void player_update(Player* p, AttackHitbox* atk, const Input* in,
                   const Level* lvl) {
    p->anim_timer++;
    if (p->hitstop > 0) {
        p->hitstop--;
        return;   /* freeze: don't process input or physics */
    }

    /* ---- timers ---- */
    if (p->attack_timer > 0)    p->attack_timer--;
    if (p->attack_cooldown > 0) p->attack_cooldown--;
    if (p->hurt_timer > 0)      p->hurt_timer--;
    if (p->coyote > 0)          p->coyote--;
    if (p->jump_buffer > 0)     p->jump_buffer--;
    if (p->sprint_iframes > 0)  p->sprint_iframes--;
    if (p->panic_lock > 0)      p->panic_lock--;

    /* kenotita passive regen */
    p->kenotita += g_tunables.kenotita_regen;
    if (p->kenotita > KENOTITA_MAX) p->kenotita = KENOTITA_MAX;

    bool grounded = on_ground(p, lvl);
    if (grounded) p->coyote = g_tunables.coyote_time;

    /* ---- mindless sprint state ---- */
    if (p->sprint_timer > 0) {
        p->sprint_timer--;
        /* locked horizontal: charge forward at sprint speed */
        p->vel.x = p->facing * g_tunables.sprint_speed;
        /* reduced gravity so the tentacles carry you */
        p->vel.y += GRAVITY * 0.3f;
        if (p->vel.y > MAX_FALL * 0.6f) p->vel.y = MAX_FALL * 0.6f;
        move_x(p, lvl, p->vel.x);
        move_y(p, lvl, p->vel.y);
        if (p->sprint_timer == 0) {
            /* end of sprint: brief vulnerability as cloak reassembles */
            p->state = grounded ? PS_IDLE : PS_FALL;
            if (p->sprint_panic) {
                /* panic sprint keeps control locked for the lock duration */
                p->panic_lock = PANIC_LOCK_TICKS;
            }
        }
        return;   /* no other input during sprint */
    }

    /* ---- panic-lock: no input allowed ---- */
    bool input_locked = (p->panic_lock > 0);

    /* ---- horizontal input ---- */
    int dir = 0;
    if (!input_locked) {
        if (in->down[BTN_LEFT])  dir -= 1;
        if (in->down[BTN_RIGHT]) dir += 1;
        if (dir != 0) p->facing = dir;
    }

    bool in_attack_anim = (p->state == PS_ATTACK || p->state == PS_POGO);
    if (!in_attack_anim) {
        float accel = grounded ? GROUND_ACCEL : AIR_ACCEL;
        float friction = grounded ? GROUND_FRICTION : AIR_FRICTION;
        if (dir != 0) {
            p->vel.x += dir * accel;
            if (p->vel.x >  g_tunables.move_speed) p->vel.x =  g_tunables.move_speed;
            if (p->vel.x < -g_tunables.move_speed) p->vel.x = -g_tunables.move_speed;
        } else {
            p->vel.x *= friction;
            if (fabsf(p->vel.x) < 0.05f) p->vel.x = 0;
        }
    } else {
        p->vel.x *= 0.92f;
    }

    /* ---- jump buffering + coyote ---- */
    if (!input_locked && in->pressed[BTN_JUMP]) p->jump_buffer = g_tunables.jump_buffer;
    bool can_jump = grounded && p->coyote > 0;
    if (p->jump_buffer > 0 && can_jump && p->state != PS_ATTACK) {
        p->vel.y = g_tunables.jump_velocity;
        p->coyote = 0;
        p->jump_buffer = 0;
        p->state = PS_JUMP;
    }
    if (!input_locked && in->released[BTN_JUMP] && p->vel.y < 0) {
        p->vel.y *= JUMP_CUT;
    }

    /* ---- gravity ---- */
    if (p->state != PS_POGO) {
        p->vel.y += g_tunables.gravity;
        if (p->vel.y > MAX_FALL) p->vel.y = MAX_FALL;
    }

    /* ---- sprint trigger (manual) ---- */
    if (!input_locked && in->pressed[BTN_SPRINT]) {
        player_try_sprint(p, in);
        if (p->state == PS_SPRINT) return;
    }

    /* ---- attack trigger ---- */
    if (!input_locked && in->pressed[BTN_ATTACK] && p->attack_cooldown == 0
        && p->state != PS_SPRINT) {
        bool want_pogo = in->down[BTN_DOWN] || in->pressed[BTN_POGO];
        if (want_pogo) {
            p->state = PS_POGO;
            p->attack_timer = ATTACK_DURATION;
            p->attack_cooldown = ATTACK_COOLDOWN;
            p->pogo_hit = false;
            atk->kind = ATTACK_POGO;
            atk->remaining_ticks = ATTACK_DURATION;
            atk->frozen = false;
            atk->freeze_ticks = 0;
            atk->freeze_total = 0;
            atk->perfect_used = false;
            atk->box = rect(p->pos.x + (PLAYER_W - 16) * 0.5f,
                            p->pos.y + PLAYER_H,
                            16.0f, ATTACK_REACH);
        } else {
            p->state = PS_ATTACK;
            p->attack_timer = ATTACK_DURATION;
            p->attack_cooldown = ATTACK_COOLDOWN;
            p->attack_hit = false;
            atk->kind = ATTACK_SLASH;
            atk->remaining_ticks = ATTACK_DURATION;
            atk->frozen = false;
            atk->freeze_ticks = 0;
            atk->freeze_total = 0;
            atk->perfect_used = false;
            float hx = (p->facing > 0) ? (p->pos.x + PLAYER_W) : (p->pos.x - ATTACK_REACH);
            atk->box = rect(hx, p->pos.y + 2, ATTACK_REACH, ATTACK_HEIGHT);
        }
    }

    /* Update attack hitbox position to follow the player. */
    if (atk->remaining_ticks > 0) {
        if (atk->kind == ATTACK_POGO) {
            atk->box.x = p->pos.x + (PLAYER_W - 16) * 0.5f;
            atk->box.y = p->pos.y + PLAYER_H;
        } else if (atk->kind == ATTACK_SLASH) {
            float hx = (p->facing > 0) ? (p->pos.x + PLAYER_W) : (p->pos.x - ATTACK_REACH);
            atk->box.x = hx;
            atk->box.y = p->pos.y + 2;
        }
        if (!atk->frozen) atk->remaining_ticks--;
        else              atk->freeze_ticks--;
    } else if (atk->kind != ATTACK_NONE) {
        atk->kind = ATTACK_NONE;
    }

    /* If the attack ended, return to movement state. */
    if (p->attack_timer == 0 && (p->state == PS_ATTACK || p->state == PS_POGO)) {
        p->state = grounded ? PS_IDLE : PS_FALL;
    }

    /* ---- integrate position with collision ---- */
    move_x(p, lvl, p->vel.x);
    bool hit_y = move_y(p, lvl, p->vel.y);

    /* pogo bounce */
    if (p->state == PS_POGO && hit_y && !p->pogo_hit && p->vel.y == 0) {
        p->vel.y = g_tunables.pogo_velocity;
        p->pogo_hit = true;
    }

    /* ---- resolve state from velocity ---- */
    if (p->state != PS_ATTACK && p->state != PS_POGO
        && p->state != PS_HURT && p->state != PS_SPRINT) {
        if (grounded) {
            if (dir != 0)       p->state = PS_WALK;
            else                p->state = PS_IDLE;
        } else {
            if (p->vel.y < 0)   p->state = PS_JUMP;
            else                p->state = PS_FALL;
        }
    }
}
