#include "enemy.h"
#include "render.h"
#include "config.h"
#include "world.h"
#include "camera.h"

#include <math.h>
#include <string.h>

/*
 * ENEMY: Void Crawler
 * -------------------
 * Patrols, lunges when player is in aggro range. Claws are parryable.
 *
 * Implemented via the Enemy vtable. All crawler-specific state lives in
 * the `data` union; we cast it to CrawlerData inside this file.
 */

typedef struct {
    int  walk_anim;        /* 0/1 for leg swap */
    int  home_x;           /* patrol anchor */
    bool had_ground;       /* for ledge detection */
} CrawlerData;

static inline CrawlerData* D(Enemy* e) {
    return (CrawlerData*)e->data.bytes;
}

/* ---- collision helpers (shared with the old enemy.c) ------------- */
static bool on_ground(const Enemy* e, const Level* lvl) {
    Rect probe = rect(e->pos.x + 1, e->pos.y + CRAWLER_H, CRAWLER_W - 2, 1);
    int tx0 = (int)(probe.x / TILE_SIZE);
    int tx1 = (int)((probe.x + probe.w - 1) / TILE_SIZE);
    int ty  = (int)((probe.y + probe.h - 1) / TILE_SIZE);
    for (int x = tx0; x <= tx1; x++) {
        if (level_is_solid(lvl, x, ty)) return true;
    }
    return false;
}

/* ledge detection: is there ground in front of my feet? */
static bool ground_ahead(const Enemy* e, const Level* lvl) {
    int tx = (e->facing > 0)
        ? (int)((e->pos.x + CRAWLER_W + 2) / TILE_SIZE)
        : (int)((e->pos.x - 2) / TILE_SIZE);
    int ty = (int)((e->pos.y + CRAWLER_H + 2) / TILE_SIZE);
    return level_is_solid(lvl, tx, ty);
}

static bool wall_ahead(const Enemy* e, const Level* lvl) {
    int tx = (e->facing > 0)
        ? (int)((e->pos.x + CRAWLER_W + 1) / TILE_SIZE)
        : (int)((e->pos.x - 1) / TILE_SIZE);
    int ty = (int)((e->pos.y + CRAWLER_H - 2) / TILE_SIZE);
    return level_is_solid(lvl, tx, ty);
}

/* ---- vtable implementations ------------------------------------- */
static void crawler_init_impl(Enemy* e, Vec2 spawn) {
    enemy_init_base(e, spawn, CRAWLER_HP);
    e->vtable = NULL;   /* set by crawler_init() below */
    CrawlerData* d = D(e);
    d->walk_anim = 0;
    d->home_x = (int)spawn.x;
    d->had_ground = true;
}

static void crawler_update(Enemy* e, Player* p, const Level* lvl) {
    CrawlerData* d = D(e);
    e->state_timer++;
    e->anim_timer++;
    if (e->hit_flash > 0) e->hit_flash--;

    /* gravity */
    e->vel.y += GRAVITY * 0.7f;
    if (e->vel.y > MAX_FALL) e->vel.y = MAX_FALL;
    e->pos.y += e->vel.y;
    {
        Rect b = rect(e->pos.x, e->pos.y, CRAWLER_W, CRAWLER_H);
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
                b = rect(e->pos.x, e->pos.y, CRAWLER_W, CRAWLER_H);
            }
        }
    }

    bool grounded = on_ground(e, lvl);
    if (!grounded) {
        e->attack_active = false;
        return;
    }

    Rect pb = player_bounds(p);
    Rect eb = rect(e->pos.x, e->pos.y, CRAWLER_W, CRAWLER_H);
    float dx = (pb.x + pb.w * 0.5f) - (eb.x + eb.w * 0.5f);
    float dy = (pb.y + pb.h * 0.5f) - (eb.y + eb.h * 0.5f);
    float dist = sqrtf(dx*dx + dy*dy);
    int toward = (dx > 0) ? 1 : -1;

    /* freeze countdown during a parry */
    if (e->attack_frozen) {
        e->freeze_ticks--;
        if (e->freeze_ticks <= 0) {
            e->attack_frozen = false;
            e->attack_active = false;
            enemy_set_state(e, ES_RECOVERY);
        }
        return;
    }

    switch (e->state) {
        case ES_IDLE:
            e->vel.x = 0;
            if (dist < CRAWLER_AGGRO_RANGE) {
                e->facing = toward;
                enemy_set_state(e, ES_WALK);
            }
            break;
        case ES_WALK: {
            e->facing = toward;
            /* LEDGE DETECTION: if no ground ahead, hop up or stop */
            if (!ground_ahead(e, lvl)) {
                /* try to hop up one tile (small step) */
                e->pos.y -= 2;
            } else if (wall_ahead(e, lvl)) {
                e->pos.y -= 2;   /* nudge up to climb small steps */
            }
            e->vel.x = toward * CRAWLER_SPEED;
            e->pos.x += e->vel.x;
            if (e->state_timer > 30) d->walk_anim ^= 1;
            if (dist < CRAWLER_ATTACK_RANGE) {
                enemy_set_state(e, ES_WINDUP);
            } else if (dist > CRAWLER_AGGRO_RANGE * 1.3f) {
                enemy_set_state(e, ES_IDLE);
            }
            break;
        }
        case ES_WINDUP:
            e->vel.x = 0;
            if (e->state_timer >= CRAWLER_ATTACK_WINDUP) {
                enemy_set_state(e, ES_ATTACK);
                e->attack_active = true;
                e->attack_hit = false;
                float ax = (e->facing > 0) ? (e->pos.x + CRAWLER_W) : (e->pos.x - 12);
                e->attack_box = rect(ax, e->pos.y + 2, 12, CRAWLER_H - 4);
            }
            break;
        case ES_ATTACK:
            e->vel.x = 0;
            {
                float ax = (e->facing > 0) ? (e->pos.x + CRAWLER_W) : (e->pos.x - 12);
                e->attack_box.x = ax;
                e->attack_box.y = e->pos.y + 2;
            }
            if (e->state_timer >= CRAWLER_ATTACK_ACTIVE) {
                e->attack_active = false;
                enemy_set_state(e, ES_RECOVERY);
            }
            break;
        case ES_RECOVERY:
            e->vel.x = 0;
            if (e->state_timer >= CRAWLER_ATTACK_RECOVERY) {
                enemy_set_state(e, dist < CRAWLER_AGGRO_RANGE ? ES_WALK : ES_IDLE);
            }
            break;
        case ES_STUN:
            e->vel.x = 0;
            if (e->state_timer >= PARRY_STUN_TICKS) {
                enemy_set_state(e, ES_IDLE);
            }
            break;
        case ES_HURT:
            e->vel.x *= 0.8f;
            e->pos.x += e->vel.x;
            if (e->state_timer >= 14) {
                enemy_set_state(e, ES_IDLE);
            }
            break;
        default:
            break;
    }
}

static void crawler_draw(Enemy* e, Renderer* r, const Camera* c) {
    CrawlerData* d = D(e);
    /* sprite is 364x364 source, displayed at CRAWLER_DISPLAY_W x CRAWLER_DISPLAY_H.
     * Center on collision box horizontally; align bottom to feet. */
    float sx = e->pos.x + (CRAWLER_W - CRAWLER_DISPLAY_W) * 0.5f;
    float sy = e->pos.y + CRAWLER_H - CRAWLER_DISPLAY_H;
    IVec2 s = camera_world_to_screen(c, sx, sy);

    SpriteID id;
    switch (e->state) {
        case ES_WINDUP:
        case ES_ATTACK:  id = SPRITE_CRAWLER_ATTACK; break;
        case ES_HURT:
        case ES_STUN:    id = SPRITE_CRAWLER_HURT;   break;
        case ES_WALK:
            id = ((e->anim_timer / 8) & 1) ? SPRITE_CRAWLER_WALK1 : SPRITE_CRAWLER_WALK2;
            (void)d;
            break;
        default:         id = SPRITE_CRAWLER_IDLE;
    }
    int flip = (e->facing < 0) ? 1 : 0;
    draw_sprite_screen_scaled(r, id, s.x, s.y,
                              CRAWLER_DISPLAY_W, CRAWLER_DISPLAY_H, flip);

    /* pulsing eye glow: crawler's eye is at ~50% across, ~55% down.
     * Pulses faster when in aggro/attack state. */
    {
        float freq = (e->state == ES_WINDUP || e->state == ES_ATTACK) ? 0.20f : 0.08f;
        float pulse = 0.5f + 0.5f * sinf(e->anim_timer * freq);
        Uint8 a = (Uint8)(40 + 80 * pulse);
        int eye_x = s.x + (int)(CRAWLER_DISPLAY_W * 0.45f);
        int eye_y = s.y + (int)(CRAWLER_DISPLAY_H * 0.50f);
        IRect eye = { eye_x, eye_y, 8, 6 };
        Color eye_col = 0x00C030FF | ((Uint32)a << 24);
        draw_rect_screen(r, eye, eye_col, 1);
    }

    /* hit flash */
    if (e->hit_flash > 0) {
        IRect rc = { s.x, s.y, CRAWLER_DISPLAY_W, CRAWLER_DISPLAY_H };
        draw_rect_screen(r, rc, 0x80FFFFFF, 1);
    }

    if (e->state == ES_STUN) {
        IRect rc = { s.x + 8, s.y - 4, 16, 8 };
        draw_rect_screen(r, rc, 0xC0FFFF40, 1);
    }
}

static void crawler_on_hit(Enemy* e, int dmg) {
    e->hp -= dmg;
    e->hit_flash = 4;
    if (e->hp <= 0) {
        e->state = ES_DEAD;
        e->attack_active = false;
        return;
    }
    enemy_set_state(e, ES_HURT);
    e->attack_active = false;
    e->vel.x = -e->facing * 2.5f;
}

static void crawler_on_parried(Enemy* e, Player* p) {
    (void)p;
    e->hp -= 2;
    e->hit_flash = 6;
    if (e->hp <= 0) {
        e->state = ES_DEAD;
    } else {
        enemy_set_state(e, ES_STUN);
    }
    e->attack_active = false;
}

static void crawler_on_death(Enemy* e, struct World* w) {
    (void)e; (void)w;
    /* no rewards in proto; particle burst is handled by combat.c */
}

static Rect crawler_attack_hitbox(const Enemy* e) {
    if (!e->attack_active) return rect(0,0,0,0);
    return e->attack_box;
}

static Rect crawler_hurtbox(const Enemy* e) {
    return rect(e->pos.x, e->pos.y, CRAWLER_W, CRAWLER_H);
}

static int crawler_sprite(const Enemy* e) {
    switch (e->state) {
        case ES_WINDUP:
        case ES_ATTACK:  return SPRITE_CRAWLER_ATTACK;
        case ES_HURT:
        case ES_STUN:    return SPRITE_CRAWLER_HURT;
        case ES_WALK:
            return ((e->anim_timer / 8) & 1) ? SPRITE_CRAWLER_WALK1 : SPRITE_CRAWLER_WALK2;
        default:         return SPRITE_CRAWLER_IDLE;
    }
}

static int crawler_sprite_flip(const Enemy* e) {
    return (e->facing < 0) ? 1 : 0;
}

static const EnemyVTable CRAWLER_VTABLE = {
    .init          = crawler_init_impl,
    .update        = crawler_update,
    .draw          = crawler_draw,
    .on_hit        = crawler_on_hit,
    .on_parried    = crawler_on_parried,
    .on_death      = crawler_on_death,
    .attack_hitbox = crawler_attack_hitbox,
    .hurtbox       = crawler_hurtbox,
    .sprite        = crawler_sprite,
    .sprite_flip   = crawler_sprite_flip,
};

void crawler_init(Enemy* e, Vec2 spawn) {
    crawler_init_impl(e, spawn);
    e->vtable = &CRAWLER_VTABLE;
}
