#ifndef TV_CAMERA_H
#define TV_CAMERA_H

#include "types.h"

typedef struct {
    Vec2 pos;            /* top-left of view in world space */
    int  w, h;           /* viewport size */

    /* shake (trauma^2 model) */
    float trauma;        /* [0,1] */
    float shake_x, shake_y;

    /* zoom (for parry drama) */
    float zoom;          /* 1.0 = normal; >1 = zoomed in on focus */
    Vec2  focus;         /* world-space point to zoom toward */
    float zoom_target;   /* eased toward this */
} Camera;

void camera_init(Camera* c, int vw, int vh);
void camera_follow(Camera* c, Vec2 target, Vec2 target_vel, int facing);
void camera_update(Camera* c);

/* Add trauma [0,1] clamped. shake = trauma^2 * SHAKE_MAX_PIXELS. */
void camera_add_trauma(Camera* c, float amount);

/* Smoothly change zoom (1.0 normal). focus is world-space center. */
void camera_set_zoom(Camera* c, float zoom, Vec2 focus);

/* Convert world-space (x,y) to screen-space (sx,sy) using current
 * shake + zoom. */
IVec2 camera_world_to_screen(const Camera* c, float x, float y);

#endif /* TV_CAMERA_H */
