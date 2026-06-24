#ifndef ART_VOIDSCREW_SPRITE_H
#define ART_VOIDSCREW_SPRITE_H

#include "../src/canvas.h"

/*
 * Voidscrew — the player's small blade. Procedural 364x364.
 * Used in HUD / inventory; in-world slash is part of player_attack_draw.
 */

static inline void voidscrew_idle_draw(Canvas* c) {
    canvas_clear(c, 0);
    /* hilt (dark wrap) */
    canvas_fill_rect_rounded(c, 150, 60, 64, 80, 8, 0xFF1F0E26);
    canvas_fill_rect_rounded(c, 160, 70, 44, 60, 6, 0xFF332B40);
    /* hilt wrap lines */
    canvas_draw_line(c, 165, 80, 199, 80, 2, 0xFF55406A);
    canvas_draw_line(c, 165, 100, 199, 100, 2, 0xFF55406A);
    canvas_draw_line(c, 165, 120, 199, 120, 2, 0xFF55406A);
    /* crossguard */
    canvas_fill_rect_rounded(c, 140, 140, 84, 20, 4, 0xFF55406A);
    /* blade (tapering triangle with glow) */
    canvas_fill_triangle(c, 150, 160, 214, 160, 182, 320, 0xFFFFFFEE);
    /* blade edge highlight */
    canvas_draw_line(c, 182, 160, 182, 320, 3, 0xFFFFFFFF);
    /* blade glow */
    canvas_fill_radial_glow(c, 182, 250, 50, 0xC030FF, 100);
    canvas_fill_radial_glow(c, 182, 300, 30, 0xFFFFFFEE, 140);
    /* tip void spark */
    canvas_fill_radial_glow(c, 182, 320, 14, 0xC030FF, 200);
}

#endif /* ART_VOIDSCREW_SPRITE_H */
