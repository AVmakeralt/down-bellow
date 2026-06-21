#ifndef TV_WORLD_H
#define TV_WORLD_H

#include "level.h"
#include "player.h"
#include "enemy.h"
#include "camera.h"
#include "render.h"

#define MAX_ENEMIES 32

typedef struct {
    Level  level;
    Player player;
    Enemy  enemies[MAX_ENEMIES];
    int    enemy_count;
    Camera camera;
    AttackHitbox player_attack;
} World;

void world_init(World* w, const char* level_path);
void world_update(World* w, const Input* in);
void world_draw(World* w, Renderer* r);

#endif /* TV_WORLD_H */
