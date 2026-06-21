#include "enemy.h"
#include "config.h"

#include <string.h>

void enemy_init_base(Enemy* e, Vec2 spawn, int hp) {
    memset(e, 0, sizeof(*e));
    e->pos = spawn;
    e->facing = -1;
    e->state = ES_IDLE;
    e->hp = hp;
    e->max_hp = hp;
}

void enemy_set_state(Enemy* e, int s) {
    e->state = s;
    e->state_timer = 0;
}

Rect enemy_default_hurtbox(const Enemy* e, int w, int h) {
    return rect(e->pos.x, e->pos.y, (float)w, (float)h);
}
