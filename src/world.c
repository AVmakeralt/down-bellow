#include "world.h"
#include "config.h"
#include "tunables.h"
#include "combat.h"
#include "camera.h"
#include "particles.h"
#include "audio.h"
#include "i18n.h"

#include <string.h>
#include <math.h>

void world_init(World* w, const char* level_path, Audio* audio) {
    memset(w, 0, sizeof(*w));
    level_load(&w->level, level_path);
    player_init(&w->player, w->level.player_spawn);
    for (int i = 0; i < w->level.enemy_count && i < MAX_ENEMIES; i++) {
        Enemy* e = &w->enemies[w->enemy_count];
        /* default: spawn as crawler. To add more types, switch on a
         * marker char in the level file (e.g. 'C' = crawler, 'M' = moth). */
        crawler_init(e, w->level.enemy_spawns[i]);
        w->enemy_count++;
    }
    camera_init(&w->camera, LOGICAL_W, LOGICAL_H);
    particles_init(&w->particles);
    tentacles_init(&w->tentacles);
    w->audio = audio;
    w->player_attack.kind = ATTACK_NONE;
}

Enemy* world_spawn_enemy(World* w) {
    if (w->enemy_count >= MAX_ENEMIES) return NULL;
    return &w->enemies[w->enemy_count++];
}

void world_update(World* w, const Input* in, float dt) {
    /* hit-stop: freeze the world while player.hitstop > 0 */
    if (w->player.hitstop > 0) {
        /* still update camera (for shake) and particles? Convention in
         * HK/Celeste is to freeze EVERYTHING. We freeze entities, but
         * let camera shake continue so the freeze itself feels physical. */
        camera_update(&w->camera);
        return;
    }

    /* clear per-tick effects */
    memset(&w->effects, 0, sizeof(w->effects));

    /* player + combat */
    player_update(&w->player, &w->player_attack, in, &w->level);

    /* panic-auto sprint: scan for overwhelming enemy presence */
    if (w->player.sprint_timer == 0 && w->player.panic_lock == 0
        && w->player.state != PS_HURT) {
        Rect pb = player_bounds(&w->player);
        int nearby = 0;
        for (int i = 0; i < w->enemy_count; i++) {
            Enemy* e = &w->enemies[i];
            if (e->state == ES_DEAD) continue;
            Rect eb = e->vtable->hurtbox(e);
            float dx = (eb.x + eb.w*0.5f) - (pb.x + pb.w*0.5f);
            float dy = (eb.y + eb.h*0.5f) - (pb.y + pb.h*0.5f);
            if (dx*dx + dy*dy < PANIC_AGGRO_RANGE * PANIC_AGGRO_RANGE) {
                nearby++;
            }
        }
        if (nearby >= 3) {
            player_force_panic_sprint(&w->player);
            w->effects.sprint_started = true;
            w->effects.last_event_pos = rect_center(pb);
            w->effects.last_event_severity = 2;
            camera_add_trauma(&w->camera, SHAKE_TRAUMA_SPRINT);
            if (w->audio) audio_play(w->audio, SFX_SPRINT);
        }
    }

    /* if sprint just started (manual), set effects */
    if (w->player.state == PS_SPRINT && w->player.sprint_timer == (g_tunables.sprint_duration_ticks - 1)) {
        w->effects.sprint_started = true;
        w->effects.last_event_pos = rect_center(player_bounds(&w->player));
        camera_add_trauma(&w->camera, SHAKE_TRAUMA_SPRINT);
        if (w->audio) audio_play(w->audio, SFX_SPRINT);
    }

    /* compact dead enemies */
    for (int i = 0; i < w->enemy_count; ) {
        if (w->enemies[i].state == ES_DEAD) {
            /* call on_death for rewards/cutscene triggers */
            if (w->enemies[i].vtable && w->enemies[i].vtable->on_death) {
                w->enemies[i].vtable->on_death(&w->enemies[i], w);
            }
            w->enemies[i] = w->enemies[w->enemy_count - 1];
            w->enemy_count--;
            continue;
        }
        i++;
    }

    for (int i = 0; i < w->enemy_count; i++) {
        Enemy* e = &w->enemies[i];
        if (e->vtable && e->vtable->update) {
            e->vtable->update(e, &w->player, &w->level);
        }
    }

    combat_resolve(&w->player, &w->player_attack, w->enemies, w->enemy_count, w);

    /* ---- react to combat effects ---- */
    if (w->effects.player_hit_enemy) {
        Vec2 pos = w->effects.last_event_pos;
        camera_add_trauma(&w->camera, g_tunables.shake_trauma_hit);
        particles_burst(&w->particles, pos, 8, PT_BLOOD,
                        vec2(-w->player.facing * 0.7f, -0.5f), 3.0f);
        particles_burst(&w->particles, pos, 4, PT_SPARK,
                        vec2(0, -0.8f), 4.0f);
        if (w->audio) audio_play(w->audio, SFX_SLASH);
    }
    if (w->effects.player_killed_enemy) {
        Vec2 pos = w->effects.last_event_pos;
        camera_add_trauma(&w->camera, SHAKE_TRAUMA_KILL);
        particles_burst(&w->particles, pos, 16, PT_VOID,
                        vec2(0, -0.3f), 4.0f);
        particles_burst(&w->particles, pos, 10, PT_BLOOD,
                        vec2(0, -0.8f), 5.0f);
        if (w->audio) audio_play(w->audio, SFX_KILL);
    }
    if (w->effects.parry_started) {
        Vec2 pos = w->effects.last_event_pos;
        camera_set_zoom(&w->camera, 1.08f, pos);
        particles_burst(&w->particles, pos, 12, PT_SPARK, vec2(0,0), 5.0f);
        if (w->audio) audio_play(w->audio, SFX_PARRY);
    }
    if (w->effects.perfect_parry) {
        Vec2 pos = w->effects.last_event_pos;
        camera_set_zoom(&w->camera, 1.15f, pos);
        camera_add_trauma(&w->camera, g_tunables.shake_trauma_perfect);
        particles_burst(&w->particles, pos, 20, PT_SPARK, vec2(0,0), 6.0f);
        particles_burst(&w->particles, pos, 8, PT_VOID, vec2(0,0), 4.0f);
        if (w->audio) audio_play(w->audio, SFX_PERFECT_PARRY);
        w->player.kenotita += KENOTITA_PERFECT_REFUND;
        if (w->player.kenotita > KENOTITA_MAX) w->player.kenotita = KENOTITA_MAX;
    }
    if (w->effects.parry_resolved) {
        camera_set_zoom(&w->camera, 1.0f, w->effects.last_event_pos);
    }
    if (w->effects.pogo_bounced) {
        Vec2 pos = w->effects.last_event_pos;
        camera_add_trauma(&w->camera, SHAKE_TRAUMA_POGO);
        particles_burst(&w->particles, pos, 8, PT_DUST, vec2(0,-0.4f), 2.5f);
        if (w->audio) audio_play(w->audio, SFX_POGO);
    }
    if (w->effects.enemy_hit_player) {
        Vec2 pos = w->effects.last_event_pos;
        camera_add_trauma(&w->camera, SHAKE_TRAUMA_HURT);
        particles_burst(&w->particles, pos, 10, PT_BLOOD,
                        vec2(w->player.facing * 0.5f, -0.3f), 3.0f);
        if (w->audio) audio_play(w->audio, SFX_HURT);
    }

    /* tentacles follow the player when sprinting */
    if (w->player.state == PS_SPRINT) {
        tentacles_update(&w->tentacles, &w->player, dt);
    }

    /* particles update */
    particles_update(&w->particles);

    /* camera follows player */
    Vec2 pcenter = vec2(w->player.pos.x + PLAYER_W * 0.5f,
                        w->player.pos.y + PLAYER_H * 0.5f);
    camera_follow(&w->camera, pcenter, w->player.vel, w->player.facing);
    camera_update(&w->camera);
}

/* ---- draw ---- */
static void draw_tiles(World* w, Renderer* r) {
    Camera* c = &w->camera;
    int tx0 = (int)(c->pos.x / TILE_SIZE);
    int ty0 = (int)(c->pos.y / TILE_SIZE);
    int tx1 = (int)((c->pos.x + c->w) / TILE_SIZE) + 1;
    int ty1 = (int)((c->pos.y + c->h) / TILE_SIZE) + 1;

    /* bgwall tiles (parallax 0.6x) — faint background detail */
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (tx < 0 || ty < 0 || tx >= w->level.w || ty >= w->level.h) continue;
            char t = w->level.tiles[ty][tx];
            if (t != '.') continue;
            int sx = (int)(tx * TILE_SIZE - c->pos.x * 0.6f + c->shake_x);
            int sy = (int)(ty * TILE_SIZE - c->pos.y * 0.6f + c->shake_y);
            draw_sprite_screen(r, SPRITE_TILE_BGWALL, sx, sy, 0);
        }
    }

    /* solid tiles (parallax 1.0x = foreground) */
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (tx < 0 || ty < 0 || tx >= w->level.w || ty >= w->level.h) continue;
            char t = w->level.tiles[ty][tx];
            if (t == '.' || t == ' ') continue;
            IVec2 s = camera_world_to_screen(c, tx * TILE_SIZE, ty * TILE_SIZE);
            SpriteID id = SPRITE_NONE;
            switch (t) {
                case '#': id = SPRITE_TILE_STONE; break;
                case 'M': id = SPRITE_TILE_MOSS;  break;
                case '=': id = SPRITE_TILE_GROUND; break;
                default: continue;
            }
            draw_sprite_screen(r, id, s.x, s.y, 0);
        }
    }
}

static void draw_player(World* w, Renderer* r) {
    Player* p = &w->player;
    Camera* c = &w->camera;
    float sx = p->pos.x + (PLAYER_W - PLAYER_SPRITE_W) * 0.5f;
    float sy = p->pos.y + PLAYER_H - PLAYER_SPRITE_H;
    IVec2 s = camera_world_to_screen(c, sx, sy);

    SpriteID id;
    switch (p->state) {
        case PS_IDLE:    id = SPRITE_PLAYER_IDLE;  break;
        case PS_WALK:
            id = ((p->anim_timer / 8) & 1) ? SPRITE_PLAYER_WALK1 : SPRITE_PLAYER_WALK2;
            break;
        case PS_JUMP:
        case PS_FALL:    id = SPRITE_PLAYER_JUMP;   break;
        case PS_ATTACK:  id = SPRITE_PLAYER_ATTACK; break;
        case PS_POGO:    id = SPRITE_PLAYER_POGO;   break;
        case PS_SPRINT:  id = SPRITE_PLAYER_ATTACK; break;
        case PS_HURT:    id = SPRITE_PLAYER_IDLE;   break;
        default:         id = SPRITE_PLAYER_IDLE;
    }
    int flip = (p->facing < 0) ? 1 : 0;

    /* idle breathing: subtle 1px vertical squash on a 60-tick cycle */
    int breath_offset = 0;
    if (p->state == PS_IDLE) {
        int phase = p->anim_timer % 60;
        if (phase < 30) breath_offset = 0;
        else            breath_offset = 1;   /* shift down 1px = "exhale" */
    }

    draw_sprite_screen(r, id, s.x, s.y + breath_offset, flip);

    /* eye pulse: cyan glow over the eyes, sine-modulated.
     * Eyes are at sprite y=10..13, x=12..20 (player_sprite.h).
     * Pulse every ~90 ticks. */
    {
        float pulse = 0.5f + 0.5f * sinf(p->anim_timer * 0.07f);
        Uint8 a = (Uint8)(50 + 90 * pulse);
        IRect eye_glow = { s.x + 12, s.y + 10 + breath_offset, 8, 4 };
        Color eye_col = 0x0088E0FF | ((Uint32)a << 24);
        draw_rect_screen(r, eye_glow, eye_col, 1);
    }

    /* hurt flash */
    if (p->hurt_timer > 0 && (p->hurt_timer & 3) == 0) {
        IRect rc = { s.x, s.y, PLAYER_SPRITE_W, PLAYER_SPRITE_H };
        draw_rect_screen(r, rc, 0x80FF2020, 1);
    }
    /* sprint i-frame shimmer */
    if (p->sprint_iframes > 0 && (p->sprint_iframes & 2) == 0) {
        IRect rc = { s.x, s.y, PLAYER_SPRITE_W, PLAYER_SPRITE_H };
        draw_rect_screen(r, rc, 0x60FFFFFF, 1);
    }
}

static void draw_enemies(World* w, Renderer* r) {
    Camera* c = &w->camera;
    for (int i = 0; i < w->enemy_count; i++) {
        Enemy* e = &w->enemies[i];
        if (e->state == ES_DEAD) continue;
        if (e->vtable && e->vtable->draw) {
            e->vtable->draw(e, r, c);
        }
    }
}

static void draw_debug(World* w, Renderer* r) {
    if (!w->debug) return;
    Camera* c = &w->camera;
    IVec2 pb = camera_world_to_screen(c, w->player.pos.x, w->player.pos.y);
    IRect pb_r = { pb.x, pb.y, PLAYER_W, PLAYER_H };
    draw_rect_screen(r, pb_r, 0xFF00FF00, 0);
    if (w->player_attack.kind != ATTACK_NONE) {
        IVec2 ab = camera_world_to_screen(c, w->player_attack.box.x, w->player_attack.box.y);
        IRect ab_r = { ab.x, ab.y, (int)w->player_attack.box.w, (int)w->player_attack.box.h };
        Color col = w->player_attack.frozen ? 0xFFFF00FF : 0xFFFFFF00;
        draw_rect_screen(r, ab_r, col, 0);
    }
    for (int i = 0; i < w->enemy_count; i++) {
        Enemy* e = &w->enemies[i];
        Rect eb = e->vtable->hurtbox(e);
        IVec2 es = camera_world_to_screen(c, eb.x, eb.y);
        IRect er = { es.x, es.y, (int)eb.w, (int)eb.h };
        draw_rect_screen(r, er, 0xFFFF4040, 0);
        if (e->attack_active) {
            IVec2 as = camera_world_to_screen(c, e->attack_box.x, e->attack_box.y);
            IRect ar = { as.x, as.y, (int)e->attack_box.w, (int)e->attack_box.h };
            Color col = e->attack_frozen ? 0xFFFF00FF : 0xFFFF8800;
            draw_rect_screen(r, ar, col, 0);
        }
    }

    /* HUD: HP, kenotita, state */
    char buf[64];
    snprintf(buf, sizeof(buf), "HP %d  Kenotita %d  State %d  Enemies %d",
             w->player.hp, (int)w->player.kenotita, w->player.state, w->enemy_count);
    (void)buf;
    /* (text rendering would go here once we have a font; for proto, skip) */
}

/* ---- vertical void gradient -------------------------------------- *
 * Replaces flat black background. Deepens from violet-black at top to
 * pure void at bottom. Sells "depth" without any art.
 */
static void draw_void_gradient(World* w, Renderer* r) {
    int h = r->logical_h;
    int w_ = r->logical_w;
    /* darker when sprinting (void absorption) */
    float darken = (w->player.state == PS_SPRINT) ? 0.5f : 1.0f;
    /* vertical gradient: top = violet-black, bottom = pure void */
    for (int y = 0; y < h; y += 2) {
        float t = (float)y / (float)h;     /* 0 top, 1 bottom */
        /* top color: 0x1A0E26 (cool void purple-black)
         * bottom color: 0x050208 (deep void) */
        Uint8 r_top = (Uint8)(0x1A * darken);
        Uint8 g_top = (Uint8)(0x0E * darken);
        Uint8 b_top = (Uint8)(0x26 * darken);
        Uint8 r_bot = (Uint8)(0x05 * darken);
        Uint8 g_bot = (Uint8)(0x02 * darken);
        Uint8 b_bot = (Uint8)(0x08 * darken);
        Uint8 rr = (Uint8)(r_top + (r_bot - r_top) * t);
        Uint8 gg = (Uint8)(g_top + (g_bot - g_top) * t);
        Uint8 bb = (Uint8)(b_top + (b_bot - b_top) * t);
        IRect rc = { 0, y, w_, 2 };
        draw_rect_screen(r, rc, 0xFF000000 | ((Uint32)bb << 16) | ((Uint32)gg << 8) | rr, 1);
    }
}

/* ---- far parallax: distant cathedral silhouettes ----------------- *
 * Pre-baked into the renderer? For now, draw 3 faint arch shapes that
 * scroll at 0.05x camera speed. Pure black with low alpha.
 */
static void draw_far_parallax(World* w, Renderer* r) {
    Camera* c = &w->camera;
    /* three arches spaced every 600 world-px, scrolling at 0.05x */
    float scroll = c->pos.x * 0.05f;
    for (int i = 0; i < 6; i++) {
        float world_x = i * 600.0f - scroll;
        int sx = (int)world_x;
        /* only draw if on-screen */
        if (sx < -120 || sx > r->logical_w + 120) continue;
        int base_y = r->logical_h - 80;
        /* draw a faint arch: rectangle base + arched top */
        Color faint = 0x50100612;   /* deep void, low alpha */
        IRect base = { sx, base_y, 80, 80 };
        draw_rect_screen(r, base, faint, 1);
        IRect top  = { sx + 8, base_y - 24, 64, 24 };
        draw_rect_screen(r, top, faint, 1);
        IRect spire = { sx + 32, base_y - 60, 16, 36 };
        draw_rect_screen(r, spire, faint, 1);
    }
}

/* ---- mid parallax: bgwall tiles at 0.4x -------------------------- */
static void draw_mid_parallax(World* w, Renderer* r) {
    Camera* c = &w->camera;
    int tx0 = (int)(c->pos.x * 0.4f / TILE_SIZE) - 1;
    int tx1 = (int)((c->pos.x * 0.4f + c->w) / TILE_SIZE) + 1;
    int ty0 = (int)(c->pos.y * 0.4f / TILE_SIZE) - 1;
    int ty1 = (int)((c->pos.y * 0.4f + c->h) / TILE_SIZE) + 1;
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            /* background tiles don't map to level data — just draw a faint
             * bgwall at every grid cell for atmosphere. */
            int sx = (int)(tx * TILE_SIZE - c->pos.x * 0.4f + c->shake_x);
            int sy = (int)(ty * TILE_SIZE - c->pos.y * 0.4f + c->shake_y);
            /* dim it: draw with low alpha by drawing a translucent dark rect over */
            draw_sprite_screen(r, SPRITE_TILE_BGWALL, sx, sy, 0);
        }
    }
}

/* ---- ambient void motes (slow upward drift) --------------------- *
 * Cheap: 80 motes scattered across the screen with sinusoidal drift.
 */
static void draw_void_motes(World* w, Renderer* r, int tick) {
    Camera* c = &w->camera;
    (void)c;
    for (int i = 0; i < 40; i++) {
        /* hash-based deterministic position per mote */
        int seed_x = i * 73;
        int seed_y = i * 131;
        float base_x = (float)((seed_x * 17) % r->logical_w);
        float base_y = (float)((seed_y * 11) % r->logical_h);
        /* drift upward, wrap */
        float y = base_y - (tick * 0.3f + i * 0.7f);
        y = fmodf(y + r->logical_h * 2, r->logical_h);
        /* sine sway */
        float x = base_x + sinf(tick * 0.02f + i) * 8.0f;
        /* twinkle alpha */
        Uint8 a = (Uint8)(40 + 40 * sinf(tick * 0.05f + i * 0.3f));
        IRect rc = { (int)x, (int)y, 1, 1 };
        Color col = 0x00C030FF | ((Uint32)a << 24);   /* signature purple */
        draw_rect_screen(r, rc, col, 1);
    }
}

void world_draw(World* w, Renderer* r) {
    /* ---- Layer 1: vertical void gradient (replaces flat clear) ---- */
    draw_void_gradient(w, r);

    /* ---- Layer 2: far parallax (cathedral silhouettes, 0.05x) ---- */
    draw_far_parallax(w, r);

    /* ---- Layer 3: mid parallax (bgwall tiles, 0.4x) -------------- */
    draw_mid_parallax(w, r);

    /* ---- Layer 4: ambient void motes (in front of mid parallax) - */
    /* use player anim_timer as the global tick */
    draw_void_motes(w, r, w->player.anim_timer);

    /* ---- Layer 5: foreground tiles + enemies + player ----------- */
    draw_tiles(w, r);
    draw_enemies(w, r);

    /* ---- Layer 6: void glow under player (signature look) ------ *
     * Intensity ramps up during combat/sprint. */
    {
        Player* p = &w->player;
        Camera* c = &w->camera;
        Vec2 pc = vec2(p->pos.x + PLAYER_W * 0.5f,
                       p->pos.y + PLAYER_H * 0.5f);
        IVec2 ps = camera_world_to_screen(c, pc.x, pc.y);
        /* base glow radius ~24px (half sprite), grows during sprint/attack */
        int radius = 24;
        Uint8 alpha = 90;
        if (p->state == PS_SPRINT) {
            radius = 48;
            alpha = 180;
        } else if (p->state == PS_ATTACK || p->state == PS_POGO) {
            radius = 36;
            alpha = 160;
        } else if (p->hurt_timer > 0) {
            radius = 32;
            alpha = 200;   /* angry red-purple when hurt */
        }
        draw_void_glow(r, ps.x, ps.y, radius, alpha);
    }

    /* tentacles behind player when sprinting */
    if (w->player.state == PS_SPRINT) {
        tentacles_draw(&w->tentacles, r, &w->camera);
    }

    draw_player(w, r);

    /* particles drawn on top */
    particles_draw(&w->particles, r, &w->camera);

    /* ---- Layer 7: post-processing (vignette + grain) ----------- */
    draw_vignette(r);
    draw_grain(r, w->player.anim_timer);

    draw_debug(w, r);
}
