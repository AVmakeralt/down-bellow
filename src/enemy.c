#include "enemy.h"
#include "config.h"
#include "level.h"

#include <math.h>
#include <string.h>

void enemy_init(Enemy* e, Vec2 spawn) {
    memset(e, 0, sizeof(*e));
    e->pos = spawn;
    e->facing = -1;
    e->state = ES_IDLE;
    e->hp = CRAWLER_HP;
}

Rect enemy_bounds(const Enemy* e) {
    return rect(e->pos.x, e->pos.y, (float)CRAWLER_W, (float)CRAWLER_H);
}

/* simple tile collision for the crawler (only horizontal, it walks on ground) */
static bool enemy_on_ground(const Enemy* e, const Level* lvl) {
    Rect probe = rect(e->pos.x + 1, e->pos.y + CRAWLER_H, CRAWLER_W - 2, 1);
    int tx0 = (int)(probe.x / TILE_SIZE);
    int tx1 = (int)((probe.x + probe.w - 1) / TILE_SIZE);
    int ty  = (int)((probe.y + probe.h - 1) / TILE_SIZE);
    for (int x = tx0; x <= tx1; x++) {
        if (level_is_solid(lvl, x, ty)) return true;
    }
    return false;
}

static bool enemy_wall_ahead(const Enemy* e, const Level* lvl) {
    int tx = (e->facing > 0)
        ? (int)((e->pos.x + CRAWLER_W + 1) / TILE_SIZE)
        : (int)((e->pos.x - 1) / TILE_SIZE);
    int ty = (int)((e->pos.y + CRAWLER_H - 2) / TILE_SIZE);
    return level_is_solid(lvl, tx, ty);
}

static void set_state(Enemy* e, EnemyState s) {
    e->state = s;
    e->state_timer = 0;
}

/* public alias for combat.c */
void enemy_set_state(Enemy* e, EnemyState s) { set_state(e, s); }

void enemy_update(Enemy* e, Player* p, const Level* lvl) {
    if (e->state == ES_DEAD) return;

    e->state_timer++;
    if (e->state_timer > 100000) e->state_timer = 0;  /* guard against overflow */

    /* gravity (always applied unless dead) */
    e->vel.y += GRAVITY * 0.7f;
    if (e->vel.y > MAX_FALL) e->vel.y = MAX_FALL;
    e->pos.y += e->vel.y;
    /* resolve Y against tiles */
    {
        Rect b = enemy_bounds(e);
        int tx0 = (int)(b.x / TILE_SIZE);
        int tx1 = (int)((b.x + b.w - 1) / TILE_SIZE);
        int ty0 = (int)(b.y / TILE_SIZE);
        int ty1 = (int)((b.y + b.h - 1) / TILE_SIZE);
        for (int ty = ty0; ty <= ty1; ty++) {
            for (int tx = tx0; tx <= tx1; tx++) {
                if (!level_is_solid(lvl, tx, ty)) continue;
                Rect tile = rect(tx*TILE_SIZE, ty*TILE_SIZE, TILE_SIZE, TILE_SIZE);
                if (!rects_overlap(b, tile)) continue;
                if (e->vel.y > 0) { e->pos.y = tile.y - CRAWLER_H; }
                else              { e->pos.y = tile.y + TILE_SIZE; }
                e->vel.y = 0;
                b = enemy_bounds(e);
            }
        }
    }

    bool grounded = enemy_on_ground(e, lvl);
    if (!grounded) {
        /* just fall, no horizontal motion */
        e->attack_active = false;
        return;
    }

    /* distance to player */
    Rect pb = player_bounds(p);
    Rect eb = enemy_bounds(e);
    float dx = (pb.x + pb.w * 0.5f) - (eb.x + eb.w * 0.5f);
    float dy = (pb.y + pb.h * 0.5f) - (eb.y + eb.h * 0.5f);
    float dist = sqrtf(dx*dx + dy*dy);
    int toward = (dx > 0) ? 1 : -1;

    /* freeze countdown during a parry */
    if (e->attack_frozen) {
        e->freeze_ticks--;
        if (e->freeze_ticks <= 0) {
            /* parry outcome resolved outside (in combat.c).
             * If we reach here without resolution, the enemy 'wins'
             * by default (clash fizzled). */
            e->attack_frozen = false;
            e->attack_active = false;
            set_state(e, ES_RECOVERY);
        }
        return;
    }

    switch (e->state) {
        case ES_IDLE:
            e->vel.x = 0;
            if (dist < CRAWLER_AGGRO_RANGE) {
                e->facing = toward;
                set_state(e, ES_WALK);
            }
            break;
        case ES_WALK: {
            /* shuffle toward player */
            e->facing = toward;
            e->vel.x = toward * CRAWLER_SPEED;
            if (enemy_wall_ahead(e, lvl)) {
                /* hop up onto a one-tile step instead of stopping */
                e->pos.y -= 2;
            }
            e->pos.x += e->vel.x;
            if (e->state_timer > 30) e->walk_anim ^= 1;
            if (dist < CRAWLER_ATTACK_RANGE) {
                set_state(e, ES_WINDUP);
            } else if (dist > CRAWLER_AGGRO_RANGE * 1.3f) {
                set_state(e, ES_IDLE);
            }
            break;
        }
        case ES_WINDUP:
            e->vel.x = 0;
            if (e->state_timer >= CRAWLER_ATTACK_WINDUP) {
                set_state(e, ES_ATTACK);
                e->attack_active = true;
                e->attack_hit = false;
                /* claws extend in facing direction */
                float ax = (e->facing > 0) ? (e->pos.x + CRAWLER_W) : (e->pos.x - 12);
                e->attack_box = rect(ax, e->pos.y + 2, 12, CRAWLER_H - 4);
            }
            break;
        case ES_ATTACK:
            e->vel.x = 0;
            /* keep hitbox attached */
            {
                float ax = (e->facing > 0) ? (e->pos.x + CRAWLER_W) : (e->pos.x - 12);
                e->attack_box.x = ax;
                e->attack_box.y = e->pos.y + 2;
            }
            if (e->state_timer >= CRAWLER_ATTACK_ACTIVE) {
                e->attack_active = false;
                set_state(e, ES_RECOVERY);
            }
            break;
        case ES_RECOVERY:
            e->vel.x = 0;
            if (e->state_timer >= CRAWLER_ATTACK_RECOVERY) {
                set_state(e, dist < CRAWLER_AGGRO_RANGE ? ES_WALK : ES_IDLE);
            }
            break;
        case ES_STUN:
            e->vel.x = 0;
            if (e->state_timer >= PARRY_STUN_TICKS) {
                set_state(e, ES_IDLE);
            }
            break;
        case ES_HURT:
            e->vel.x *= 0.8f;
            e->pos.x += e->vel.x;
            if (e->state_timer >= 14) {
                set_state(e, ES_IDLE);
            }
            break;
        case ES_DEAD:
            break;
    }
}
