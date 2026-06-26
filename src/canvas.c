#include "canvas.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void canvas_clear(Canvas* c, uint32_t color) {
    for (int i = 0; i < CANVAS_W * CANVAS_H; i++) c->px[i] = color;
}

static inline void set_px(Canvas* c, int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= CANVAS_W || y >= CANVAS_H) return;
    c->px[y * CANVAS_W + x] = color;
}

/* alpha-over blend: src over dst, src has its own alpha */
static inline uint32_t blend_over(uint32_t dst, uint32_t src) {
    uint8_t sa = (src >> 24) & 0xFF;
    if (sa == 0) return dst;
    if (sa == 255) return src;
    uint8_t da = (dst >> 24) & 0xFF;
    uint8_t sr = src & 0xFF, sg = (src >> 8) & 0xFF, sb = (src >> 16) & 0xFF;
    uint8_t dr = dst & 0xFF, dg = (dst >> 8) & 0xFF, db = (dst >> 16) & 0xFF;
    float a = sa / 255.0f;
    float inv = 1.0f - a;
    uint8_t out_r = (uint8_t)(sr * a + dr * inv);
    uint8_t out_g = (uint8_t)(sg * a + dg * inv);
    uint8_t out_b = (uint8_t)(sb * a + db * inv);
    uint8_t out_a = (uint8_t)(sa + da * inv);
    return (uint32_t)out_r | ((uint32_t)out_g << 8) |
           ((uint32_t)out_b << 16) | ((uint32_t)out_a << 24);
}

uint32_t canvas_blend(uint32_t dst, uint32_t src) { return blend_over(dst, src); }

void canvas_fill_rect(Canvas* c, int x, int y, int w, int h, uint32_t color) {
    bool opaque = ((color >> 24) == 0xFF);
    for (int yy = y; yy < y + h; yy++) {
        if (yy < 0 || yy >= CANVAS_H) continue;
        for (int xx = x; xx < x + w; xx++) {
            if (xx < 0 || xx >= CANVAS_W) continue;
            if (opaque) {
                c->px[yy * CANVAS_W + xx] = color;
            } else {
                c->px[yy * CANVAS_W + xx] = blend_over(c->px[yy * CANVAS_W + xx], color);
            }
        }
    }
}

void canvas_fill_circle(Canvas* c, int cx, int cy, int r, uint32_t color) {
    if (r <= 0) return;
    int r2 = r * r;
    bool opaque = ((color >> 24) == 0xFF);
    for (int y = cy - r; y <= cy + r; y++) {
        if (y < 0 || y >= CANVAS_H) continue;
        for (int x = cx - r; x <= cx + r; x++) {
            if (x < 0 || x >= CANVAS_W) continue;
            int dx = x - cx, dy = y - cy;
            if (dx*dx + dy*dy <= r2) {
                if (opaque) {
                    c->px[y * CANVAS_W + x] = color;
                } else {
                    c->px[y * CANVAS_W + x] = blend_over(c->px[y * CANVAS_W + x], color);
                }
            }
        }
    }
}

void canvas_fill_ellipse(Canvas* c, int cx, int cy, int rx, int ry, uint32_t color) {
    if (rx <= 0 || ry <= 0) return;
    bool opaque = ((color >> 24) == 0xFF);
    for (int y = cy - ry; y <= cy + ry; y++) {
        if (y < 0 || y >= CANVAS_H) continue;
        for (int x = cx - rx; x <= cx + rx; x++) {
            if (x < 0 || x >= CANVAS_W) continue;
            float dx = (float)(x - cx) / (float)rx;
            float dy = (float)(y - cy) / (float)ry;
            if (dx*dx + dy*dy <= 1.0f) {
                if (opaque) {
                    c->px[y * CANVAS_W + x] = color;
                } else {
                    c->px[y * CANVAS_W + x] = blend_over(c->px[y * CANVAS_W + x], color);
                }
            }
        }
    }
}

void canvas_fill_rect_rounded(Canvas* c, int x, int y, int w, int h, int radius, uint32_t color) {
    /* fill the rect, then carve corners by overdrawing with transparent —
     * simpler: fill central rect + 4 edge rects + 4 corner circles */
    canvas_fill_rect(c, x + radius, y, w - 2*radius, h, color);
    canvas_fill_rect(c, x, y + radius, radius, h - 2*radius, color);
    canvas_fill_rect(c, x + w - radius, y + radius, radius, h - 2*radius, color);
    canvas_fill_circle(c, x + radius, y + radius, radius, color);
    canvas_fill_circle(c, x + w - radius - 1, y + radius, radius, color);
    canvas_fill_circle(c, x + radius, y + h - radius - 1, radius, color);
    canvas_fill_circle(c, x + w - radius - 1, y + h - radius - 1, radius, color);
}

void canvas_fill_vgradient(Canvas* c, int x, int y, int w, int h,
                           uint32_t top, uint32_t bot) {
    for (int yy = 0; yy < h; yy++) {
        float t = (h > 1) ? (float)yy / (float)(h - 1) : 0.0f;
        uint8_t tr = top & 0xFF, tg = (top >> 8) & 0xFF, tb = (top >> 16) & 0xFF, ta = (top >> 24) & 0xFF;
        uint8_t br = bot & 0xFF, bg = (bot >> 8) & 0xFF, bb = (bot >> 16) & 0xFF, ba = (bot >> 24) & 0xFF;
        uint8_t r = (uint8_t)(tr + (br - tr) * t);
        uint8_t g = (uint8_t)(tg + (bg - tg) * t);
        uint8_t b = (uint8_t)(tb + (bb - tb) * t);
        uint8_t a = (uint8_t)(ta + (ba - ta) * t);
        uint32_t col = (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | ((uint32_t)a << 24);
        for (int xx = 0; xx < w; xx++) {
            set_px(c, x + xx, y + yy, col);
        }
    }
}

void canvas_fill_hgradient(Canvas* c, int x, int y, int w, int h,
                           uint32_t left, uint32_t right) {
    for (int xx = 0; xx < w; xx++) {
        float t = (w > 1) ? (float)xx / (float)(w - 1) : 0.0f;
        uint8_t lr = left & 0xFF, lg = (left >> 8) & 0xFF, lb = (left >> 16) & 0xFF, la = (left >> 24) & 0xFF;
        uint8_t rr = right & 0xFF, rg = (right >> 8) & 0xFF, rb = (right >> 16) & 0xFF, ra = (right >> 24) & 0xFF;
        uint8_t r = (uint8_t)(lr + (rr - lr) * t);
        uint8_t g = (uint8_t)(lg + (rg - lg) * t);
        uint8_t b = (uint8_t)(lb + (rb - lb) * t);
        uint8_t a = (uint8_t)(la + (ra - la) * t);
        uint32_t col = (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | ((uint32_t)a << 24);
        for (int yy = 0; yy < h; yy++) {
            set_px(c, x + xx, y + yy, col);
        }
    }
}

void canvas_fill_radial_glow(Canvas* c, int cx, int cy, int r,
                             uint32_t rgb, uint8_t max_alpha) {
    if (r <= 0) return;
    uint8_t cr = rgb & 0xFF, cg = (rgb >> 8) & 0xFF, cb = (rgb >> 16) & 0xFF;
    for (int y = cy - r; y <= cy + r; y++) {
        if (y < 0 || y >= CANVAS_H) continue;   /* skip OOB rows */
        for (int x = cx - r; x <= cx + r; x++) {
            if (x < 0 || x >= CANVAS_W) continue;   /* skip OOB cols */
            float dx = (float)(x - cx);
            float dy = (float)(y - cy);
            float d = sqrtf(dx*dx + dy*dy) / (float)r;
            if (d > 1.0f) continue;
            float t = 1.0f - d;
            t = t * t * t;   /* ease-out cubic */
            uint8_t a = (uint8_t)(max_alpha * t);
            uint32_t src = (uint32_t)cr | ((uint32_t)cg << 8) |
                           ((uint32_t)cb << 16) | ((uint32_t)a << 24);
            c->px[y * CANVAS_W + x] = blend_over(c->px[y * CANVAS_W + x], src);
        }
    }
}

void canvas_draw_line(Canvas* c, int x0, int y0, int x1, int y1,
                      int thickness, uint32_t color) {
    /* Bresenham + thickness via circle stamp at each step */
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int x = x0, y = y0;
    int half = thickness / 2;
    while (true) {
        canvas_fill_circle(c, x, y, half, color);
        if (x == x1 && y == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 <  dx) { err += dx; y += sy; }
    }
}

void canvas_fill_triangle(Canvas* c, int x0, int y0, int x1, int y1,
                          int x2, int y2, uint32_t color) {
    /* bounding-box + barycentric test */
    int min_x = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int max_x = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int min_y = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int max_y = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            /* barycentric */
            float denom = (float)((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
            if (denom == 0) continue;
            float a = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / denom;
            float b = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / denom;
            float cc = 1.0f - a - b;
            if (a >= 0 && b >= 0 && cc >= 0) {
                set_px(c, x, y, color);
            }
        }
    }
}
