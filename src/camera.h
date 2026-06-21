#ifndef TV_CAMERA_H
#define TV_CAMERA_H

#include "types.h"

typedef struct {
    Vec2 pos;        /* top-left of view in world space */
    int  w, h;       /* viewport size */
} Camera;

void camera_init(Camera* c, int vw, int vh);
void camera_follow(Camera* c, Vec2 target, Vec2 target_vel, int facing);

#endif /* TV_CAMERA_H */
