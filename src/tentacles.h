#ifndef TV_TENTACLES_H
#define TV_TENTACLES_H

#include "types.h"
#include "config.h"
#include "player.h"
#include "render.h"
#include "camera.h"

/*
 * Mindless Sprint tentacles.
 *
 * When the player is sprinting, 3-4 pitch-black void tentacles sprout
 * from their torso. Each tentacle is a 5-segment distance-constraint
 * chain that trails behind the player and lashes at the environment.
 *
 * Visuals: drawn as decaying black circles per segment.
 * Gameplay: yank small enemies into the player's wake; smash fragile
 *           tiles (TODO once we have a "fragile" tile flag); i-frames
 *           on sprint start.
 *
 * Math is the standard FABRIK-style distance constraint: each segment
 * tries to stay TENTACLE_SEGMENT_LEN pixels from the previous one,
 * with a sine-wave offset so they thrash.
 */

typedef struct {
    Vec2 segments[TENTACLE_SEGMENTS];
    float phase;            /* for sine-wave thrashing */
    int   side;             /* -1 or +1: which side of the player */
} Tentacle;

typedef struct {
    Tentacle t[4];          /* up to 4 tentacles */
    int      count;         /* active count (3 normally, 4 on perfect sprint) */
    float    elapsed;       /* seconds since sprint start, for thrash freq */
} TentacleRig;

void tentacles_init(TentacleRig* rig);
void tentacles_update(TentacleRig* rig, const Player* p, float dt);
void tentacles_draw(const TentacleRig* rig, Renderer* r, const Camera* c);

#endif /* TV_TENTACLES_H */
