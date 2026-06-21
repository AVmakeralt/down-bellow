#ifndef TV_TYPES_H
#define TV_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Core scalar / struct types shared across the engine.
 * Keep this file dependency-free so anything can include it.
 */

typedef struct { float x, y; } Vec2;
typedef struct { int x, y; } IVec2;
typedef struct { float x, y, w, h; } Rect;     /* world-space AABB */
typedef struct { int   x, y, w, h; } IRect;

static inline Vec2  vec2(float x, float y)            { Vec2 v={x,y}; return v; }
static inline Rect  rect(float x,float y,float w,float h){ Rect r={x,y,w,h}; return r; }

static inline bool rects_overlap(Rect a, Rect b) {
    return a.x < b.x + b.w
        && a.x + a.w > b.x
        && a.y < b.y + b.h
        && a.y + a.h > b.y;
}

#endif /* TV_TYPES_H */
