#ifndef TV_ENEMY_H
#define TV_ENEMY_H

#include <stdbool.h>
#include "types.h"
#include "level.h"
#include "player.h"
#include "render.h"
#include "camera.h"

/*
 * Enemy framework with a function-pointer vtable.
 *
 * Each enemy type (Crawler, Moth, Lost, Keres, future bosses) lives in its
 * own .c file and provides a const EnemyVTable* describing its behavior.
 * The shared Enemy struct carries only the generic state every enemy
 * needs: position, velocity, facing, HP, state machine index, timers,
 * hurtbox, and the attack hitbox (parryable).
 *
 * The World owns a single pool of Enemy slots. Spawning a new enemy is:
 *
 *     Enemy* e = world_spawn_enemy(&world);
 *     crawler_init(e, spawn_pos);
 *
 * where crawler_init() sets e->vtable = &crawler_vtable and initializes
 * type-specific data stored in the `data` union.
 *
 * All combat/world code calls through the vtable: e->vtable->update(e,...),
 * e->vtable->on_hit(e, dmg), etc.
 */

#define ENEMY_DATA_BYTES 128   /* per-enemy type-specific scratch space */

typedef enum {
    ES_IDLE = 0,
    ES_WALK,
    ES_WINDUP,
    ES_ATTACK,
    ES_RECOVERY,
    ES_STUN,
    ES_HURT,
    ES_DEAD,
    /* bosses can use values >= 100 for their own state machine */
    ES_CUSTOM = 100
} EnemyState;

typedef struct Enemy Enemy;
struct World;   /* forward decl; full definition in world.h */

typedef struct {
    void (*init)(Enemy* self, Vec2 spawn);
    void (*update)(Enemy* self, Player* p, const Level* lvl);
    void (*draw)(Enemy* self, Renderer* r, const Camera* c);
    void (*on_hit)(Enemy* self, int dmg);          /* normal hit, no parry */
    void (*on_parried)(Enemy* self, Player* p);    /* lost a parry */
    void (*on_death)(Enemy* self, struct World* w);  /* drop rewards, trigger cutscene */
    Rect (*attack_hitbox)(const Enemy* self);      /* current parryable hitbox, or empty */
    Rect (*hurtbox)(const Enemy* self);            /* body box for taking damage */
    int  (*sprite)(const Enemy* self);             /* SpriteID for current frame */
    int  (*sprite_flip)(const Enemy* self);        /* 1 if flipped horizontally */
} EnemyVTable;

struct Enemy {
    Vec2 pos;                /* top-left of collision box */
    Vec2 vel;
    int  facing;             /* +1 right, -1 left */
    int  state;              /* EnemyState (or ES_CUSTOM+ for bosses) */
    int  state_timer;
    int  hp;
    int  max_hp;

    int  anim_timer;         /* incremented every update; drives walk anims */

    /* attack hitbox (parryable). Active only when attack_active is true. */
    Rect attack_box;
    bool attack_active;
    bool attack_frozen;      /* mid-parry: hitbox frozen */
    int  freeze_ticks;
    bool attack_hit;         /* already damaged player with this swing */

    /* for hit-stop + screenshake hooks when this enemy takes a hit */
    int  hit_flash;          /* > 0 = tinted white for hit_flash ticks */

    const EnemyVTable* vtable;

    /* type-specific scratch (e.g. Crawler patrol home, Moth flight phase) */
    union {
        char bytes[ENEMY_DATA_BYTES];
        void* ptr;
    } data;
};

/* Generic helpers shared by all enemy types */
void enemy_init_base(Enemy* e, Vec2 spawn, int hp);
void enemy_set_state(Enemy* e, int s);
Rect enemy_default_hurtbox(const Enemy* e, int w, int h);

/* The Crawler-specific init (defined in enemy_crawler.c) */
void crawler_init(Enemy* e, Vec2 spawn);

#endif /* TV_ENEMY_H */
