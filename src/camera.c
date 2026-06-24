#include "camera.h"
#include "config.h"
#include "tunables.h"

#include <math.h>
#include <stdlib.h>

void camera_init(Camera* c, int vw, int vh) {
    c->pos = vec2(0, 0);
    c->w = vw;
    c->h = vh;
    c->trauma = 0;
    c->shake_x = c->shake_y = 0;
    c->zoom = 1.0f;
    c->zoom_target = 1.0f;
    c->focus = vec2(0, 0);
}

void camera_follow(Camera* c, Vec2 target, Vec2 target_vel, int facing) {
    (void)target_vel;
    float desired_x = target.x - c->w * 0.5f + facing * CAMERA_LOOKAHEAD;
    float desired_y = target.y - c->h * 0.5f;

    float dx = desired_x - c->pos.x;
    float dy = desired_y - c->pos.y;
    if (fabsf(dx) < CAMERA_DEADZONE_X) dx = 0;
    if (fabsf(dy) < CAMERA_DEADZONE_Y) dy = 0;

    c->pos.x += dx * CAMERA_LERP;
    c->pos.y += dy * CAMERA_LERP;

    if (c->pos.x < 0) c->pos.x = 0;
    if (c->pos.y < 0) c->pos.y = 0;
}

void camera_update(Camera* c) {
    /* decay trauma exponentially */
    c->trauma *= g_tunables.shake_decay;
    if (c->trauma < 0.001f) c->trauma = 0;

    /* shake = trauma^2 * max (squared for punchiness) */
    float shake = c->trauma * c->trauma * g_tunables.shake_max_pixels;
    /* random offset, scaled by shake; use a quick LCG so it's deterministic
     * per-tick if we ever need replay */
    c->shake_x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * shake;
    c->shake_y = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * shake;

    /* ease zoom toward target */
    c->zoom += (c->zoom_target - c->zoom) * 0.18f;
}

void camera_add_trauma(Camera* c, float amount) {
    c->trauma += amount;
    if (c->trauma > 1.0f) c->trauma = 1.0f;
}

void camera_set_zoom(Camera* c, float zoom, Vec2 focus) {
    c->zoom_target = zoom;
    c->focus = focus;
}

/* world-to-screen with shake + zoom about focus point */
IVec2 camera_world_to_screen(const Camera* c, float x, float y) {
    float sx = x - c->pos.x + c->shake_x;
    float sy = y - c->pos.y + c->shake_y;
    if (c->zoom != 1.0f) {
        /* zoom about focus: translate so focus is at viewport center,
         * scale, translate back. */
        float fcx = c->focus.x - c->pos.x + c->shake_x;
        float fcy = c->focus.y - c->pos.y + c->shake_y;
        sx = (sx - fcx) * c->zoom + fcx;
        sy = (sy - fcy) * c->zoom + fcy;
    }
    IVec2 r = { (int)sx, (int)sy };
    return r;
}
