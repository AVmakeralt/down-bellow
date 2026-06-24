#ifndef TV_TUNABLES_H
#define TV_TUNABLES_H

#include <stdbool.h>

/*
 * Runtime-tunable mirror of the most-tweaked config.h values.
 * Loaded from tunables.txt at startup, hot-reloaded via inotify.
 *
 * Why both config.h #defines AND a Tunables struct?
 *   - config.h values are baked into array sizes and switch statements;
 *     they need to be compile-time.
 *   - Tunables are runtime; you can change them mid-playtest without
 *     recompiling. Use these for any value you actually iterate on.
 *
 * tunables.txt format (one per line, "key value"):
 *
 *     gravity           0.55
 *     move_speed        2.4
 *     jump_velocity    -9.5
 *     coyote_time       6
 *     jump_buffer       6
 *     pogo_velocity   -11.0
 *     parry_freeze_ticks 20
 *     parry_perfect_window 4
 *     parry_stun_ticks  40
 *     hitstop_hit       3
 *     hitstop_parry     8
 *     hitstop_perfect  14
 *     shake_max_pixels  8.0
 *     shake_decay       0.86
 *     sprint_speed      8.0
 *     kenotita_regen    0.15
 *
 * Missing keys fall back to the compile-time defaults below.
 */

typedef struct {
    float gravity;
    float move_speed;
    float jump_velocity;
    int   coyote_time;
    int   jump_buffer;
    float pogo_velocity;

    int   parry_freeze_ticks;
    int   parry_perfect_window;
    int   parry_stun_ticks;

    int   hitstop_hit;
    int   hitstop_parry;
    int   hitstop_perfect;
    int   hitstop_kill;

    float shake_max_pixels;
    float shake_decay;
    float shake_trauma_hit;
    float shake_trauma_parry;
    float shake_trauma_perfect;

    float sprint_speed;
    int   sprint_duration_ticks;
    float kenotita_regen;
    int   kenotita_sprint_cost;
} Tunables;

extern Tunables g_tunables;   /* global, single-instance for simplicity */

void tunables_load_defaults(Tunables* t);
bool tunables_load_file(Tunables* t, const char* path);
/* Try to reload; if parse fails, keep prior values and return false. */
bool tunables_reload(Tunables* t, const char* path);

#endif /* TV_TUNABLES_H */
