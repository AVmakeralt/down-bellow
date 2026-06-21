#include "level.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool level_load(Level* lvl, const char* path) {
    memset(lvl, 0, sizeof(*lvl));
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "level_load: cannot open %s\n", path);
        return false;
    }
    char line[512];
    int y = 0;
    int w = 0;
    while (fgets(line, sizeof(line), f)) {
        /* strip trailing newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0) continue;
        if ((int)len > MAX_LEVEL_W) len = MAX_LEVEL_W;
        if (y >= MAX_LEVEL_H) break;
        if (y == 0) w = (int)len;
        /* pad short lines with spaces so the grid is rectangular */
        memcpy(lvl->tiles[y], line, len);
        for (int x = (int)len; x < w; x++) lvl->tiles[y][x] = ' ';
        lvl->tiles[y][w] = '\0';

        /* scan for spawn markers */
        for (int x = 0; x < w; x++) {
            char c = lvl->tiles[y][x];
            if (c == 'P') {
                lvl->player_spawn = vec2(x * TILE_SIZE + (TILE_SIZE - PLAYER_W) * 0.5f,
                                         y * TILE_SIZE + (TILE_SIZE - PLAYER_H));
                lvl->tiles[y][x] = ' ';
            } else if (c == 'E') {
                if (lvl->enemy_count < 32) {
                    lvl->enemy_spawns[lvl->enemy_count] = vec2(
                        x * TILE_SIZE + (TILE_SIZE - CRAWLER_W) * 0.5f,
                        y * TILE_SIZE + (TILE_SIZE - CRAWLER_H));
                    lvl->enemy_count++;
                }
                lvl->tiles[y][x] = ' ';
            }
        }
        y++;
    }
    fclose(f);
    lvl->w = w;
    lvl->h = y;
    return true;
}

bool level_is_solid(const Level* lvl, int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= lvl->w || ty >= lvl->h) {
        /* out of bounds horizontally = solid wall; out of bounds vertically
         * below = solid floor (so player can't fall forever); above = empty */
        if (ty >= lvl->h) return true;
        if (ty < 0) return false;
        return true;  /* horizontal OOB acts as wall */
    }
    char c = lvl->tiles[ty][tx];
    return c == '#' || c == 'M' || c == '=';
}

char level_at(const Level* lvl, int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= lvl->w || ty >= lvl->h) return ' ';
    return lvl->tiles[ty][tx];
}
