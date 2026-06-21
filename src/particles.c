#include "particles.h"
#include "render.h"
#include "camera.h"
#include "config.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ABGR palette for particle kinds */
#define COL_SPARK   0xFFFFF0A0
#define COL_DUST    0xFFA8A090
#define COL_VOID    0xFFA42D7A
#define COL_BLOOD   0xFF2020A8   /* ABGR: A=20, B=20, G=20, R=A8 -> red */
#define COL_SHARD   0xFFFFFFFF

static Color kind_color(ParticleKind k) {
    switch (k) {
        case PT_SPARK:  return COL_SPARK;
        case PT_DUST:   return COL_DUST;
        case PT_VOID:   return COL_VOID;
        case PT_BLOOD:  return COL_BLOOD;
        case PT_SHARD:  return COL_SHARD;
    }
    return COL_DUST;
}

static int kind_life(ParticleKind k) {
    switch (k) {
        case PT_SPARK:  return 14;
        case PT_DUST:   return 30;
        case PT_VOID:   return 40;
        case PT_BLOOD:  return 24;
        case PT_SHARD:  return 60;
    }
    return 20;
}

static float kind_size(ParticleKind k) {
    switch (k) {
        case PT_SPARK:  return 2.0f;
        case PT_DUST:   return 3.0f;
        case PT_VOID:   return 4.0f;
        case PT_BLOOD:  return 2.0f;
        case PT_SHARD:  return 5.0f;
    }
    return 2.0f;
}

void particles_init(ParticleSystem* ps) {
    memset(ps, 0, sizeof(*ps));
}

static int find_free_slot(ParticleSystem* ps) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ps->particles[i].active) return i;
    }
    return -1;
}

static void spawn_one(ParticleSystem* ps, Vec2 pos, Vec2 vel,
                      ParticleKind kind, int life, float size, Color col) {
    int slot = find_free_slot(ps);
    if (slot < 0) return;
    Particle* p = &ps->particles[slot];
    p->active = true;
    p->kind = kind;
    p->pos = pos;
    p->vel = vel;
    p->life = life;
    p->max_life = life;
    p->size = size;
    p->color = col;
}

void particles_burst(ParticleSystem* ps, Vec2 origin, int count,
                     ParticleKind kind, Vec2 bias_dir, float speed) {
    Color col = kind_color(kind);
    int   life = kind_life(kind);
    float size = kind_size(kind);
    for (int i = 0; i < count; i++) {
        /* random direction in a circle, biased by bias_dir */
        float a = (rand() / (float)RAND_MAX) * (float)(2.0 * M_PI);
        Vec2 dir = { cosf(a), sinf(a) };
        /* blend 50% random + 50% bias */
        Vec2 v;
        v.x = (dir.x + bias_dir.x) * 0.5f * speed * (0.5f + 0.5f * (rand() / (float)RAND_MAX));
        v.y = (dir.y + bias_dir.y) * 0.5f * speed * (0.5f + 0.5f * (rand() / (float)RAND_MAX));
        spawn_one(ps, origin, v, kind, life, size, col);
    }
}

void particles_cone(ParticleSystem* ps, Vec2 origin, int count,
                    ParticleKind kind, float dir_radians,
                    float spread_radians, float speed) {
    Color col = kind_color(kind);
    int   life = kind_life(kind);
    float size = kind_size(kind);
    for (int i = 0; i < count; i++) {
        float t = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;   /* [-1,1] */
        float a = dir_radians + t * spread_radians * 0.5f;
        float s = speed * (0.6f + 0.4f * (rand() / (float)RAND_MAX));
        Vec2 v = { cosf(a) * s, sinf(a) * s };
        spawn_one(ps, origin, v, kind, life, size, col);
    }
}

void particles_update(ParticleSystem* ps) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* p = &ps->particles[i];
        if (!p->active) continue;
        p->pos.x += p->vel.x;
        p->pos.y += p->vel.y;
        /* kind-specific physics */
        switch (p->kind) {
            case PT_BLOOD:
            case PT_SHARD:
                p->vel.y += 0.30f;        /* gravity */
                p->vel.x *= 0.96f;
                break;
            case PT_DUST:
                p->vel.y -= 0.05f;        /* slow rise */
                p->vel.x *= 0.92f;
                p->vel.y *= 0.92f;
                break;
            case PT_VOID:
                p->vel.x *= 0.98f;
                p->vel.y *= 0.98f;
                break;
            case PT_SPARK:
                p->vel.x *= 0.85f;
                p->vel.y *= 0.85f;
                break;
        }
        p->life--;
        if (p->life <= 0) p->active = false;
    }
}

void particles_draw(const ParticleSystem* ps, Renderer* r, const Camera* c) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        const Particle* p = &ps->particles[i];
        if (!p->active) continue;
        IVec2 s = camera_world_to_screen(c, p->pos.x, p->pos.y);
        /* fade based on life remaining */
        float t = (float)p->life / (float)p->max_life;
        Uint8 a = (Uint8)(255 * (t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t)));
        Color col = p->color;
        /* swap alpha */
        col = (col & 0x00FFFFFF) | ((uint32_t)a << 24);
        int size = (int)(p->size * (0.4f + 0.6f * t));
        if (size < 1) size = 1;
        IRect rc = { s.x - size, s.y - size, size * 2, size * 2 };
        draw_rect_screen(r, rc, col, 1);
    }
}
