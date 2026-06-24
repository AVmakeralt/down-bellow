#ifndef TV_WORLD_H
#define TV_WORLD_H

#include "level.h"
#include "player.h"
#include "enemy.h"
#include "camera.h"
#include "render.h"
#include "particles.h"
#include "audio.h"
#include "tentacles.h"
#include "save.h"

#define MAX_ENEMIES 64   /* mirrors config.h */

/*
 * World holds the live game state for one loaded level. Combat, particles,
 * audio, camera, and the enemy pool all live here.
 *
 * Effects struct: events that happened during the latest combat_resolve
 * tick, surfaced so the main loop / scene stack can react (e.g. play a
 * sound, add screen shake, advance hit-stop). These are set inside
 * combat.c and consumed by world_update().
 */

typedef struct {
    bool  player_hit_enemy;
    bool  player_killed_enemy;
    bool  enemy_hit_player;
    bool  parry_started;
    bool  parry_resolved;
    bool  perfect_parry;
    bool  pogo_bounced;
    bool  sprint_started;
    bool  saved;                 /* a save fired this tick (auto or manual) */
    SaveReason save_reason;      /* why (for the save-flash label) */
    Vec2  last_event_pos;        /* world-space, for camera zoom + particles */
    int   last_event_severity;   /* 1=hit, 2=parry, 3=perfect, 4=kill */
} CombatEffects;

typedef struct World {
    Level          level;
    Player         player;
    Enemy          enemies[MAX_ENEMIES];
    int            enemy_count;
    Camera         camera;
    AttackHitbox   player_attack;
    ParticleSystem particles;
    TentacleRig    tentacles;
    Audio*         audio;          /* borrowed from main; not owned */
    CombatEffects  effects;
    bool           debug;

    /* ---- save system ---- */
    SaveData       save;           /* in-memory mirror of save.dat         */
    uint32_t       play_time_ticks;/* total ticks this session + loaded    */
    uint32_t       last_save_tick; /* tick of last save (for cooldown)     */
    int            save_flash;     /* >0 = draw save indicator, counts down */
    int            save_zone_idx;  /* -1 = not in a zone; else index into level.save_points */
    bool           save_zone_armed;/* re-arm after leaving a zone          */
    char           level_path[256];/* current level path (for save_capture)*/
} World;

void world_init(World* w, const char* level_path, Audio* audio);
void world_update(World* w, const Input* in, float dt);
void world_draw(World* w, Renderer* r);

/* Spawn a fresh enemy slot; returns NULL if pool is full. Caller must
 * immediately call the type-specific init (e.g. crawler_init(e, spawn)). */
Enemy* world_spawn_enemy(World* w);

/* Force a save now (used by manual F5 + autosave triggers). Returns true
 * if the save actually wrote. */
bool world_save_now(World* w, SaveReason reason);

/* Check if the player is overlapping any save point. Returns the index
 * or -1 if none. */
int  world_current_save_zone(const World* w);

#endif /* TV_WORLD_H */
