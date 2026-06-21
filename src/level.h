#ifndef TV_LEVEL_H
#define TV_LEVEL_H

#include <stdbool.h>
#include "types.h"

/*
 * Level layout: ASCII tilemap loaded from levels/ (txt files).
 *
 * Tile legend (matches art/town_tiles.h):
 *   '#'  stone       (solid)
 *   'M'  moss cap    (solid)
 *   '='  ground      (solid)
 *   '.'  bgwall      (non-solid, parallax background)
 *   ' '  empty       (non-solid, no draw)
 *   'P'  player spawn
 *   'E'  enemy spawn (void crawler)
 *
 * Lines must be equal width. The first line is the top of the world.
 */

#define MAX_LEVEL_W 256
#define MAX_LEVEL_H 64

typedef struct {
    int   w, h;
    char  tiles[MAX_LEVEL_H][MAX_LEVEL_W + 1];   /* +1 for NUL */
    Vec2  player_spawn;
    Vec2  enemy_spawns[32];
    int   enemy_count;
} Level;

bool level_load(Level* lvl, const char* path);
bool level_is_solid(const Level* lvl, int tx, int ty);
char level_at(const Level* lvl, int tx, int ty);

#endif /* TV_LEVEL_H */
