#include "camera.h"
#include "config.h"

#include <math.h>

void camera_init(Camera* c, int vw, int vh) {
    c->pos = vec2(0, 0);
    c->w = vw;
    c->h = vh;
}

void camera_follow(Camera* c, Vec2 target, Vec2 target_vel, int facing) {
    (void)target_vel;  /* reserved for future velocity-based lookahead */
    /* desired camera position centers the target with lookahead */
    float desired_x = target.x - c->w * 0.5f + facing * CAMERA_LOOKAHEAD;
    float desired_y = target.y - c->h * 0.5f;

    /* deadzone: only move if target drifts past a threshold */
    float dx = desired_x - c->pos.x;
    float dy = desired_y - c->pos.y;
    if (fabsf(dx) < CAMERA_DEADZONE_X) dx = 0;
    if (fabsf(dy) < CAMERA_DEADZONE_Y) dy = 0;

    c->pos.x += dx * CAMERA_LERP;
    c->pos.y += dy * CAMERA_LERP;

    /* clamp so we never show negative space (assuming the world starts at 0,0) */
    if (c->pos.x < 0) c->pos.x = 0;
    if (c->pos.y < 0) c->pos.y = 0;
}
