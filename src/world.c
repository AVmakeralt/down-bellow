#include "world.h"
#include "config.h"
#include "combat.h"

#include <string.h>
#include <math.h>

void world_init(World* w, const char* level_path) {
    memset(w, 0, sizeof(*w));
    level_load(&w->level, level_path);
    player_init(&w->player, w->level.player_spawn);
    for (int i = 0; i < w->level.enemy_count && i < MAX_ENEMIES; i++) {
        enemy_init(&w->enemies[i], w->level.enemy_spawns[i]);
    }
    w->enemy_count = w->level.enemy_count;
    camera_init(&w->camera, WINDOW_W, WINDOW_H);
    w->player_attack.kind = ATTACK_NONE;
}

void world_update(World* w, const Input* in) {
    player_update(&w->player, &w->player_attack, in, &w->level);

    /* compact dead enemies so we don't keep drawing them */
    for (int i = 0; i < w->enemy_count; ) {
        if (w->enemies[i].state == ES_DEAD) {
            w->enemies[i] = w->enemies[w->enemy_count - 1];
            w->enemy_count--;
            continue;
        }
        i++;
    }
    for (int i = 0; i < w->enemy_count; i++) {
        enemy_update(&w->enemies[i], &w->player, &w->level);
    }

    combat_resolve(&w->player, &w->player_attack, w->enemies, w->enemy_count);

    Vec2 pcenter = vec2(w->player.pos.x + PLAYER_W * 0.5f,
                        w->player.pos.y + PLAYER_H * 0.5f);
    camera_follow(&w->camera, pcenter, w->player.vel, w->player.facing);
}

/* draw the visible tiles plus the player + enemies, applying camera offset */
static void draw_tiles(World* w, Renderer* r) {
    Camera* c = &w->camera;
    int tx0 = (int)(c->pos.x / TILE_SIZE);
    int ty0 = (int)(c->pos.y / TILE_SIZE);
    int tx1 = (int)((c->pos.x + c->w) / TILE_SIZE) + 1;
    int ty1 = (int)((c->pos.y + c->h) / TILE_SIZE) + 1;
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (tx < 0 || ty < 0 || tx >= w->level.w || ty >= w->level.h) continue;
            char t = w->level.tiles[ty][tx];
            if (t == ' ' ) continue;
            int sx = (int)(tx * TILE_SIZE - c->pos.x);
            int sy = (int)(ty * TILE_SIZE - c->pos.y);
            SpriteID id = SPRITE_NONE;
            switch (t) {
                case '#': id = SPRITE_TILE_STONE; break;
                case 'M': id = SPRITE_TILE_MOSS;  break;
                case '=': id = SPRITE_TILE_GROUND; break;
                case '.': id = SPRITE_TILE_BGWALL; break;
                default: continue;
            }
            draw_sprite_screen(r, id, sx, sy, 0);
        }
    }
}

static void draw_player(World* w, Renderer* r) {
    Player* p = &w->player;
    Camera* c = &w->camera;
    /* sprite is 32x32 but collision box is 18x26. Center the sprite on the
     * collision box horizontally and align feet to the bottom. */
    float sx = p->pos.x + (PLAYER_W - PLAYER_SPRITE_W) * 0.5f - c->pos.x;
    float sy = p->pos.y + PLAYER_H - PLAYER_SPRITE_H - c->pos.y;

    SpriteID id;
    switch (p->state) {
        case PS_IDLE:    id = SPRITE_PLAYER_IDLE;  break;
        case PS_WALK:
            /* 6-tick swap */
            id = ((SDL_GetTicks() / 80) & 1) ? SPRITE_PLAYER_WALK1 : SPRITE_PLAYER_WALK2;
            break;
        case PS_JUMP:
        case PS_FALL:    id = SPRITE_PLAYER_JUMP;   break;
        case PS_ATTACK:  id = SPRITE_PLAYER_ATTACK; break;
        case PS_POGO:    id = SPRITE_PLAYER_POGO;   break;
        case PS_HURT:    id = SPRITE_PLAYER_IDLE;   break;
        default:         id = SPRITE_PLAYER_IDLE;
    }
    int flip = (p->facing < 0) ? 1 : 0;
    draw_sprite(r, id, sx, sy, flip);

    /* hurt flash: tint the player by drawing a red rect over them */
    if (p->hurt_timer > 0 && (p->hurt_timer & 3) == 0) {
        IRect rc = { (int)sx, (int)sy, PLAYER_SPRITE_W, PLAYER_SPRITE_H };
        draw_rect_screen(r, rc, 0x80FF2020, 1);
    }
}

static void draw_enemies(World* w, Renderer* r) {
    Camera* c = &w->camera;
    for (int i = 0; i < w->enemy_count; i++) {
        Enemy* e = &w->enemies[i];
        if (e->state == ES_DEAD) continue;
        float sx = e->pos.x + (CRAWLER_W - CRAWLER_SPRITE_W) * 0.5f - c->pos.x;
        float sy = e->pos.y + CRAWLER_H - CRAWLER_SPRITE_H - c->pos.y;
        SpriteID id;
        switch (e->state) {
            case ES_WINDUP:
            case ES_ATTACK:  id = SPRITE_CRAWLER_ATTACK; break;
            case ES_HURT:
            case ES_STUN:    id = SPRITE_CRAWLER_HURT;   break;
            case ES_WALK:
                id = ((SDL_GetTicks() / 120) & 1) ? SPRITE_CRAWLER_WALK1 : SPRITE_CRAWLER_WALK2;
                break;
            default:         id = SPRITE_CRAWLER_IDLE;
        }
        int flip = (e->facing < 0) ? 1 : 0;
        draw_sprite(r, id, sx, sy, flip);

        if (e->state == ES_STUN) {
            /* draw a stun sparkle ring */
            IRect rc = { (int)sx + 8, (int)sy - 4, 16, 8 };
            draw_rect_screen(r, rc, 0xC0FFFF40, 1);
        }
    }
}

static void draw_debug(World* w, Renderer* r) {
    if (!r->debug) return;
    Camera* c = &w->camera;
    /* player collision box */
    IRect pb = { (int)(w->player.pos.x - c->pos.x),
                 (int)(w->player.pos.y - c->pos.y),
                 PLAYER_W, PLAYER_H };
    draw_rect_screen(r, pb, 0xFF00FF00, 0);
    /* player attack hitbox */
    if (w->player_attack.kind != ATTACK_NONE) {
        IRect ab = { (int)(w->player_attack.box.x - c->pos.x),
                     (int)(w->player_attack.box.y - c->pos.y),
                     (int)w->player_attack.box.w, (int)w->player_attack.box.h };
        draw_rect_screen(r, ab, w->player_attack.frozen ? 0xFFFF00FF : 0xFFFFFF00, 0);
    }
    /* enemies */
    for (int i = 0; i < w->enemy_count; i++) {
        Enemy* e = &w->enemies[i];
        IRect eb = { (int)(e->pos.x - c->pos.x),
                     (int)(e->pos.y - c->pos.y),
                     CRAWLER_W, CRAWLER_H };
        draw_rect_screen(r, eb, 0xFFFF4040, 0);
        if (e->attack_active) {
            IRect ab = { (int)(e->attack_box.x - c->pos.x),
                         (int)(e->attack_box.y - c->pos.y),
                         (int)e->attack_box.w, (int)e->attack_box.h };
            draw_rect_screen(r, ab, e->attack_frozen ? 0xFFFF00FF : 0xFFFF8800, 0);
        }
    }
}

void world_draw(World* w, Renderer* r) {
    /* Background: ash-foot town dusk */
    renderer_clear(r, 0xFF120C14);  /* deep void purple-black */
    draw_tiles(w, r);
    draw_enemies(w, r);
    draw_player(w, r);
    draw_debug(w, r);
}
