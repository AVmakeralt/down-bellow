#ifndef TV_PARTICLES_H
#define TV_PARTICLES_H

#include "types.h"
#include "render.h"
#include "camera.h"

/*
 * Particle pool. Fixed-size, free-slot list, no allocations per-frame.
 * Used for: slash trails, pogo dust, parry sparks, void bursts, blood,
 * sprint tentacle residue, anchor shatter debris.
 */

#define MAX_PARTICLES 256   /* mirrors config.h */

typedef enum {
    PT_SPARK = 0,    /* bright, fast, short life  (parry/spark) */
    PT_DUST,         /* tan/grey, slow rise        (pogo dust)   */
    PT_VOID,         /* purple, drifts + fades     (sprint trail)*/
    PT_BLOOD,        /* red, gravity, splats       (enemy hit)   */
    PT_SHARD,        /* white, gravity, big        (anchor)      */
} ParticleKind;

typedef struct {
    bool  active;
    ParticleKind kind;
    Vec2  pos;
    Vec2  vel;
    int   life;        /* ticks remaining */
    int   max_life;    /* ticks at spawn (for fade) */
    float size;        /* pixel radius */
    Color color;       /* ABGR */
} Particle;

typedef struct {
    Particle particles[MAX_PARTICLES];
} ParticleSystem;

void particles_init(ParticleSystem* ps);

/* Spawn N particles at (x,y) with given kind and direction bias. */
void particles_burst(ParticleSystem* ps, Vec2 origin, int count,
                     ParticleKind kind, Vec2 bias_dir, float speed);

/* Spawn a directed cone of particles (e.g. slash trail). */
void particles_cone(ParticleSystem* ps, Vec2 origin, int count,
                    ParticleKind kind, float dir_radians,
                    float spread_radians, float speed);

void particles_update(ParticleSystem* ps);
void particles_draw(const ParticleSystem* ps, Renderer* r, const Camera* c);

#endif /* TV_PARTICLES_H */
