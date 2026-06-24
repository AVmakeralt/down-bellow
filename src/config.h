#ifndef TV_CONFIG_H
#define TV_CONFIG_H

/*
 * The Voided - gameplay tunables.
 *
 * Two layers:
 *   1. compile-time #defines (this file) — used for array sizes, enum-like
 *      constants, anything that needs to be known at compile time.
 *   2. runtime Tunables struct (tunables.h/.c) — mirrors the float/int
 *      values below and is loaded from tunables.txt at startup, then
 *      hot-reloaded via inotify. Use Tunables* t for any value that
 *      you want to tweak without recompiling.
 *
 * All units are PIXELS and SECONDS. The fixed-timestep runs at 60 Hz so
 * 1 tick = 1/60 s. Velocities are pixels per tick (not per second) for
 * stable physics regardless of render framerate.
 */

/* --- window / render --------------------------------------------- */
#define WINDOW_W           1280
#define WINDOW_H           720
/* Internal render resolution. All draw calls happen in this space; SDL
 * upscales to WINDOW_W x WINDOW_H with nearest-neighbor. Lower = chunkier
 * pixels + bigger sprites relative to screen. 480x270 = 16:9 at 1/8 area. */
#define LOGICAL_W          480
#define LOGICAL_H          270
#define RENDER_SCALE       1
#define TARGET_FPS         60
#define FIXED_HZ           60

/* --- camera ----------------------------------------------------- */
#define CAMERA_LERP        0.18f
#define CAMERA_LOOKAHEAD   80.0f
#define CAMERA_DEADZONE_X  120.0f
#define CAMERA_DEADZONE_Y  80.0f

/* --- world / tiles ---------------------------------------------- */
#define TILE_SIZE          32

/* --- player physics --------------------------------------------- */
#define PLAYER_W           18
#define PLAYER_H           26
#define PLAYER_SPRITE_W    32
#define PLAYER_SPRITE_H    32
/* Display size for procedural 364x364 sprites. The source canvas is
 * 364x364; we downscale to these dimensions for the painted look.
 * Bigger = more visible detail. 96x128 = 47% of 270px screen height. */
#define PLAYER_DISPLAY_W   96
#define PLAYER_DISPLAY_H   128

#define GRAVITY            0.55f
#define MAX_FALL           16.0f
#define MOVE_SPEED         2.4f
#define AIR_ACCEL          0.30f
#define GROUND_ACCEL       0.60f
#define GROUND_FRICTION    0.78f
#define AIR_FRICTION       0.94f

#define JUMP_VELOCITY      -9.5f
#define JUMP_CUT           0.45f
#define COYOTE_TIME        6
#define JUMP_BUFFER        6

#define POGO_VELOCITY      -11.0f
#define POGO_NEEDS_DOWN    true

/* --- combat: voidslash ----------------------------------------- */
#define ATTACK_DURATION    14
#define ATTACK_COOLDOWN    18
#define ATTACK_REACH       30.0f
#define ATTACK_HEIGHT      22.0f

/* --- combat: parry --------------------------------------------- *
 * First PARRY_PERFECT_WINDOW ticks of the freeze = "perfect parry".
 * Perfect parry deals bonus damage + refunds kenotita + free void burst.
 */
#define PARRY_FREEZE_TICKS 20
#define PARRY_PERFECT_WINDOW 4
#define PARRY_STUN_TICKS   40

/* --- hit-stop --------------------------------------------------- *
 * On every successful hit (player -> enemy or enemy -> player), freeze
 * the world for HITSTOP_TICKS frames. Scaled by event severity in code.
 */
#define HITSTOP_TICKS_HIT       3
#define HITSTOP_TICKS_PARRY     8
#define HITSTOP_TICKS_PERFECT   14
#define HITSTOP_TICKS_KILL      20
#define HITSTOP_TICKS_BOSS_HIT  6

/* --- screen shake ---------------------------------------------- *
 * trauma in [0,1], decays exponentially each tick.
 * shake_offset = max_shake * trauma * trauma  (squared for punchiness)
 */
#define SHAKE_MAX_PIXELS    8.0f
#define SHAKE_DECAY         0.86f      /* per-tick multiplier */
#define SHAKE_TRAUMA_HIT    0.20f
#define SHAKE_TRAUMA_PARRY  0.45f
#define SHAKE_TRAUMA_PERFECT 0.75f
#define SHAKE_TRAUMA_POGO   0.30f
#define SHAKE_TRAUMA_HURT   0.60f
#define SHAKE_TRAUMA_KILL   0.80f
#define SHAKE_TRAUMA_SPRINT 0.15f
#define SHAKE_TRAUMA_ANCHOR 1.0f

/* --- particles ------------------------------------------------- */
#define MAX_PARTICLES       256

/* --- mindless sprint ------------------------------------------ *
 * kenotita = void resource spent to manually trigger sprint.
 * Panic-auto trigger is free but locks control for PANIC_LOCK_TICKS.
 */
#define KENOTITA_MAX        100
#define KENOTITA_REGEN      0.15f     /* per tick */
#define KENOTITA_SPRINT_COST 30       /* per manual activation */
#define KENOTITA_PERFECT_REFUND 25
#define SPRINT_DURATION_TICKS 30      /* length of sprint */
#define SPRINT_SPEED        8.0f
#define SPRINT_IFRAME_TICKS 6         /* intangible at start of sprint */
#define PANIC_AGGRO_RANGE   220.0f    /* enemies within this range force panic */
#define PANIC_LOCK_TICKS    90        /* 1.5s loss of control */
#define TENTACLE_SEGMENTS   5
#define TENTACLE_SEGMENT_LEN 9.0f

/* --- enemy: void crawler --------------------------------------- */
#define CRAWLER_W          24
#define CRAWLER_H          14
#define CRAWLER_SPRITE_W   32
#define CRAWLER_SPRITE_H   32
#define CRAWLER_DISPLAY_W  96
#define CRAWLER_DISPLAY_H  96
#define CRAWLER_SPEED      0.6f
#define CRAWLER_HP         3
#define CRAWLER_AGGRO_RANGE 180.0f
#define CRAWLER_ATTACK_RANGE 36.0f
#define CRAWLER_ATTACK_WINDUP 18
#define CRAWLER_ATTACK_ACTIVE 12
#define CRAWLER_ATTACK_RECOVERY 24

/* --- world pools ----------------------------------------------- */
#define MAX_ENEMIES        64

#endif /* TV_CONFIG_H */
