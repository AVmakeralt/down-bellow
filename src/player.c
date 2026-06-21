#include "player.h"
#include "config.h"
#include "level.h"

#include <math.h>
#include <string.h>

void player_init(Player* p, Vec2 spawn) {
    memset(p, 0, sizeof(*p));
    p->pos = spawn;
    p->facing = 1;
    p->state = PS_IDLE;
    p->hp = 5;
    p->coyote = 0;
    p->jump_buffer = 0;
    p->attack_timer = 0;
    p->attack_cooldown = 0;
    p->hurt_timer = 0;
}

Rect player_bounds(const Player* p) {
    return rect(p->pos.x, p->pos.y, (float)PLAYER_W, (float)PLAYER_H);
}

/* --- collision helpers ------------------------------------------- *
 * Axis-separated tile collision. Move X then Y, resolving each axis
 * independently so we never clip through thin walls.
 */
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
    /* Probe one pixel below the feet. */
    Rect probe = rect(p->pos.x + 1, p->pos.y + PLAYER_H, PLAYER_W - 2, 1);
    int tx0 = (int)(probe.x / TILE_SIZE);
    int tx1 = (int)((probe.x + probe.w - 1) / TILE_SIZE);
    int ty  = (int)((probe.y + probe.h - 1) / TILE_SIZE);
    for (int x = tx0; x <= tx1; x++) {
        if (level_is_solid(lvl, x, ty)) return true;
    }
    return false;
}

void player_update(Player* p, AttackHitbox* atk, const Input* in,
                   const Level* lvl) {
    /* ---- timers ---- */
    if (p->attack_timer > 0)    p->attack_timer--;
    if (p->attack_cooldown > 0) p->attack_cooldown--;
    if (p->hurt_timer > 0)      p->hurt_timer--;
    if (p->coyote > 0)          p->coyote--;
    if (p->jump_buffer > 0)     p->jump_buffer--;

    bool grounded = on_ground(p, lvl);
    if (grounded) p->coyote = COYOTE_TIME;

    /* ---- horizontal input ---- */
    int dir = 0;
    if (in->down[BTN_LEFT])  dir -= 1;
    if (in->down[BTN_RIGHT]) dir += 1;
    if (dir != 0) p->facing = dir;

    bool in_attack_anim = (p->state == PS_ATTACK || p->state == PS_POGO);
    if (!in_attack_anim) {
        float accel = grounded ? GROUND_ACCEL : AIR_ACCEL;
        float friction = grounded ? GROUND_FRICTION : AIR_FRICTION;
        if (dir != 0) {
            p->vel.x += dir * accel;
            if (p->vel.x >  MOVE_SPEED) p->vel.x =  MOVE_SPEED;
            if (p->vel.x < -MOVE_SPEED) p->vel.x = -MOVE_SPEED;
        } else {
            p->vel.x *= friction;
            if (fabsf(p->vel.x) < 0.05f) p->vel.x = 0;
        }
    } else {
        /* slight air control during attack */
        p->vel.x *= 0.92f;
    }

    /* ---- jump buffering + coyote ---- */
    if (in->pressed[BTN_JUMP]) p->jump_buffer = JUMP_BUFFER;
    bool can_jump = grounded && p->coyote > 0;
    if (p->jump_buffer > 0 && can_jump && p->state != PS_ATTACK) {
        p->vel.y = JUMP_VELOCITY;
        p->coyote = 0;
        p->jump_buffer = 0;
        p->state = PS_JUMP;
    }
    /* jump cut */
    if (in->released[BTN_JUMP] && p->vel.y < 0) {
        p->vel.y *= JUMP_CUT;
    }

    /* ---- gravity ---- */
    if (p->state != PS_POGO) {
        p->vel.y += GRAVITY;
        if (p->vel.y > MAX_FALL) p->vel.y = MAX_FALL;
    }

    /* ---- attack trigger ---- *
     * Two flavors:
     *   - BTN_ATTACK while BTN_DOWN held  -> pogo (downward slash)
     *   - BTN_ATTACK otherwise            -> horizontal slash
     */
    if (in->pressed[BTN_ATTACK] && p->attack_cooldown == 0) {
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
            atk->resolved = false;
            /* pogo hitbox extends DOWN from the player's feet */
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
            atk->resolved = false;
            /* slash hitbox extends in facing direction */
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

    /* pogo bounce: if we pogo'd and hit something below, bounce. */
    if (p->state == PS_POGO && hit_y && !p->pogo_hit && p->vel.y == 0) {
        p->vel.y = POGO_VELOCITY;
        p->pogo_hit = true;
    }

    /* ---- resolve state from velocity ---- */
    if (p->state != PS_ATTACK && p->state != PS_POGO && p->state != PS_HURT) {
        if (grounded) {
            if (dir != 0)       p->state = PS_WALK;
            else                p->state = PS_IDLE;
        } else {
            if (p->vel.y < 0)   p->state = PS_JUMP;
            else                p->state = PS_FALL;
        }
    }
}
