#ifndef TV_CONFIG_H
#define TV_CONFIG_H

/*
 * The Voided - gameplay tunables. Tweak freely.
 *
 * All units are PIXELS and SECONDS. The fixed-timestep runs at 60 Hz so
 * 1 tick = 1/60 s. Velocities are pixels per tick (not per second) for
 * stable physics regardless of render framerate.
 *
 * To add a new tunable: declare it here as #define, then use it in code.
 */

/* --- window / render --------------------------------------------- */
#define WINDOW_W           1280
#define WINDOW_H           720
#define RENDER_SCALE       1            /* 1 = 1:1 pixel scale, 2 = chunky */
#define TARGET_FPS         60
#define FIXED_HZ           60

/* --- camera ----------------------------------------------------- */
#define CAMERA_LERP        0.18f        /* 0 = locked, 1 = instant */
#define CAMERA_LOOKAHEAD   80.0f        /* px ahead in facing direction */
#define CAMERA_DEADZONE_X  120.0f
#define CAMERA_DEADZONE_Y  80.0f

/* --- world / tiles ---------------------------------------------- */
#define TILE_SIZE          32           /* px; matches art/town_tiles.h  */

/* --- player physics --------------------------------------------- */
#define PLAYER_W           18           /* collision box (smaller than 32 sprite) */
#define PLAYER_H           26
#define PLAYER_SPRITE_W    32
#define PLAYER_SPRITE_H    32

#define GRAVITY            0.55f        /* px/tick^2 */
#define MAX_FALL           16.0f        /* terminal velocity */
#define MOVE_SPEED         2.4f
#define AIR_ACCEL          0.30f
#define GROUND_ACCEL       0.60f
#define GROUND_FRICTION    0.78f        /* 1.0 = no friction */
#define AIR_FRICTION       0.94f

#define JUMP_VELOCITY      -9.5f
#define JUMP_CUT           0.45f        /* velocity multiplier on release */
#define COYOTE_TIME        6            /* ticks after leaving ground you can still jump */
#define JUMP_BUFFER        6            /* ticks before landing where jump still triggers */

#define POGO_VELOCITY      -11.0f       /* bounce from downward slash */
#define POGO_NEEDS_DOWN    true         /* require holding down to pogo */

/* --- combat: voidslash ----------------------------------------- */
#define ATTACK_DURATION    14           /* ticks the slash hitbox is active */
#define ATTACK_COOLDOWN    18           /* ticks before next slash */
#define ATTACK_REACH       30.0f        /* px in front of player */
#define ATTACK_HEIGHT      22.0f

/* --- combat: parry --------------------------------------------- *
 * When the player's voidslash hitbox overlaps an enemy's attack hitbox:
 *   - both hitboxes freeze for PARRY_FREEZE_TICKS
 *   - during the freeze, if the player's hitbox is in range, the player wins
 *     (enemy is stunned). If only the enemy is in range when freeze ends,
 *     the enemy wins (player takes a hit).
 *   - if neither is in range at end of freeze, both attacks fizzle.
 */
#define PARRY_FREEZE_TICKS 20
#define PARRY_WINDOW_TICKS 20           /* same as freeze; exposed for clarity */
#define PARRY_STUN_TICKS   40           /* enemy stunned after losing parry */

/* --- enemy: void crawler --------------------------------------- */
#define CRAWLER_W          24
#define CRAWLER_H          14
#define CRAWLER_SPRITE_W   32
#define CRAWLER_SPRITE_H   32
#define CRAWLER_SPEED      0.6f
#define CRAWLER_HP         3
#define CRAWLER_AGGRO_RANGE 180.0f
#define CRAWLER_ATTACK_RANGE 36.0f
#define CRAWLER_ATTACK_WINDUP 18
#define CRAWLER_ATTACK_ACTIVE 12
#define CRAWLER_ATTACK_RECOVERY 24

#endif /* TV_CONFIG_H */
