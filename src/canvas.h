#ifndef TV_CANVAS_H
#define TV_CANVAS_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Canvas: a 364x364 ABGR pixel buffer with drawing primitives.
 *
 * Instead of hand-authoring 132,496 grid characters per sprite frame,
 * we compose each sprite from C function calls: fill_circle, fill_ellipse,
 * radial_glow, vgradient, etc. This is still "coded art" — no PNGs — but
 * scales to high detail (364x364) without exploding the header size.
 *
 * Each sprite is a function:
 *
 *     void player_idle_draw(Canvas* c) {
 *         canvas_clear(c, 0);
 *         canvas_fill_ellipse(c, 182, 340, 60, 12, 0x40000000);  // shadow
 *         canvas_fill_vgradient(c, 110, 130, 144, 180,           // cloak
 *                               0xFF332B40, 0xFF1F0E26);
 *         canvas_fill_radial_glow(c, 165, 125, 14, 0x88E0FF, 255); // eye
 *         ...
 *     }
 *
 * At startup, render.c bakes each canvas into an SDL_Texture.
 */

#define CANVAS_W 364
#define CANVAS_H 364

typedef struct {
    uint32_t px[CANVAS_W * CANVAS_H];   /* ABGR8888, alpha-premultiplied not required */
} Canvas;

/* ---- primitives ---- */
void canvas_clear(Canvas* c, uint32_t color);
void canvas_fill_rect(Canvas* c, int x, int y, int w, int h, uint32_t color);
void canvas_fill_rect_rounded(Canvas* c, int x, int y, int w, int h, int radius, uint32_t color);
void canvas_fill_circle(Canvas* c, int cx, int cy, int r, uint32_t color);
void canvas_fill_ellipse(Canvas* c, int cx, int cy, int rx, int ry, uint32_t color);
void canvas_fill_vgradient(Canvas* c, int x, int y, int w, int h,
                           uint32_t top, uint32_t bot);
void canvas_fill_hgradient(Canvas* c, int x, int y, int w, int h,
                           uint32_t left, uint32_t right);
/* Radial glow: additive-feeling circular gradient. rgb is the color (alpha
 * ignored); max_alpha is the center alpha; falls off to 0 at radius. */
void canvas_fill_radial_glow(Canvas* c, int cx, int cy, int r,
                             uint32_t rgb, uint8_t max_alpha);
/* Thick line between two points. */
void canvas_draw_line(Canvas* c, int x0, int y0, int x1, int y1,
                      int thickness, uint32_t color);
/* Triangle (filled). */
void canvas_fill_triangle(Canvas* c, int x0, int y0, int x1, int y1,
                          int x2, int y2, uint32_t color);

/* ---- helpers ---- */
/* Blend src (with given alpha) over dst. Used by glow primitives. */
uint32_t canvas_blend(uint32_t dst, uint32_t src_with_alpha);

#endif /* TV_CANVAS_H */
