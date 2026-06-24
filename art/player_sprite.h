#ifndef ART_PLAYER_SPRITE_H
#define ART_PLAYER_SPRITE_H

#include "../src/canvas.h"

/*
 * The Voided (player) — procedural 364x364 sprite, composed from C drawing
 * primitives. No more 32x32 grid strings; this gives us real detail (cloak
 * folds, glowing eyes, horn curve) at any display scale.
 *
 * Palette (mirrors art/palette.h):
 *   VOID_BLACK     0xFF12060A
 *   CLOAK_GREY     0xFF332B40
 *   CLOAK_HI       0xFF55406A
 *   CLOAK_SHADOW   0xFF1F0E26
 *   EYE_WHITE      0xFFEAE0F0
 *   EYE_CYAN       0xFF88E0FF
 *   VOID_PURPLE    0xFF2A0A3A
 *   VOID_BRIGHT    0xFFC030FF
 *   VOIDSCREW_GLOW 0xFFFFFFEE
 *
 * Each frame is a function: void player_<frame>_draw(Canvas* c).
 * The canvas is 364x364; the character is roughly centered and stands
 * ~260px tall (from y=60 hood top to y=320 feet).
 *
 * Frames: idle, walk1, walk2, jump, attack, pogo
 */

#define PV_VOID_BLACK     0xFF12060A
#define PV_CLOAK_GREY     0xFF332B40
#define PV_CLOAK_HI       0xFF55406A
#define PV_CLOAK_SHADOW   0xFF1F0E26
#define PV_EYE_WHITE      0xFFEAE0F0
#define PV_EYE_CYAN       0xFF88E0FF
#define PV_VOID_PURPLE    0xFF2A0A3A
#define PV_VOID_BRIGHT    0xFFC030FF
#define PV_VOIDSCREW_GLOW 0xFFFFFFEE

/* shared body-drawing helper: cloak, hood, horn, eyes.
 * legs_y = top of the legs (varies by frame for breathing/walk)
 * arm_offset = horizontal offset for hands (varies by frame) */
static inline void player_draw_body(Canvas* c, int legs_y, int arm_offset, bool glow_eyes) {
    /* ---- shadow under feet ---- */
    canvas_fill_ellipse(c, 182, 330, 55, 10, 0x60000000);

    /* ---- cloak body: tapered trapezoid (narrow top, wide bottom) ---- *
     * Draw the cloak as a filled trapezoid using two triangles, then overlay
     * a vertical gradient clipped to the same shape. */
    int cloak_top_x0 = 135, cloak_top_x1 = 229;   /* top corners  */
    int cloak_bot_x0 = 115, cloak_bot_x1 = 249;   /* bottom corners (wider) */
    int cloak_top_y = 110;
    int cloak_bot_y = legs_y + 60;
    /* fill trapezoid with cloak color first */
    canvas_fill_triangle(c, cloak_top_x0, cloak_top_y,
                            cloak_bot_x0, cloak_bot_y,
                            cloak_bot_x1, cloak_bot_y, PV_CLOAK_GREY);
    canvas_fill_triangle(c, cloak_top_x0, cloak_top_y,
                            cloak_top_x1, cloak_top_y,
                            cloak_bot_x1, cloak_bot_y, PV_CLOAK_GREY);
    /* overlay gradient (semi-transparent so it tints the cloak) */
    canvas_fill_triangle(c, cloak_top_x0, cloak_top_y,
                            cloak_bot_x0, cloak_bot_y,
                            cloak_bot_x1, cloak_bot_y, 0xC055406A);
    canvas_fill_triangle(c, cloak_top_x0, cloak_top_y,
                            cloak_top_x1, cloak_top_y,
                            cloak_bot_x1, cloak_bot_y, 0xC055406A);
    /* bottom shadow gradient */
    canvas_fill_ellipse(c, 182, cloak_bot_y, 67, 20, PV_CLOAK_SHADOW);

    /* ---- cloak rim light (bright left edge — void aura catching light) ---- */
    canvas_draw_line(c, cloak_top_x0, cloak_top_y + 5,
                     cloak_bot_x0, cloak_bot_y - 5, 4, PV_CLOAK_HI);
    canvas_draw_line(c, cloak_top_x0 + 1, cloak_top_y + 5,
                     cloak_bot_x0 + 1, cloak_bot_y - 5, 2, 0xFF8868A0);

    /* ---- hood (large pointed ellipse on top) ---- */
    canvas_fill_ellipse(c, 182, 95, 68, 60, PV_CLOAK_GREY);
    /* hood highlight (top-left catches light) */
    canvas_fill_ellipse(c, 165, 80, 30, 25, PV_CLOAK_HI);
    /* hood inner shadow (face cavity opening) */
    canvas_fill_ellipse(c, 182, 105, 40, 32, PV_VOID_BLACK);

    /* ---- horn (disfigured, curving up-right from hood) ---- */
    canvas_draw_line(c, 205, 65, 240, 25, 10, PV_CLOAK_HI);
    canvas_draw_line(c, 240, 25, 258, 15, 7, PV_CLOAK_HI);
    canvas_fill_circle(c, 205, 65, 6, PV_CLOAK_HI);
    canvas_fill_circle(c, 258, 15, 5, PV_CLOAK_HI);
    /* horn tip void glow */
    canvas_fill_radial_glow(c, 260, 13, 12, PV_VOID_BRIGHT, 200);

    /* ---- eyes (two glowing cyan ovals deep in the hood) ---- */
    if (glow_eyes) {
        /* large outer glow */
        canvas_fill_radial_glow(c, 168, 100, 22, PV_EYE_CYAN, 180);
        canvas_fill_radial_glow(c, 196, 100, 22, PV_EYE_CYAN, 180);
        /* bright white core */
        canvas_fill_ellipse(c, 168, 100, 7, 9, PV_EYE_WHITE);
        canvas_fill_ellipse(c, 196, 100, 7, 9, PV_EYE_WHITE);
        /* cyan center */
        canvas_fill_ellipse(c, 168, 100, 4, 6, PV_EYE_CYAN);
        canvas_fill_ellipse(c, 196, 100, 4, 6, PV_EYE_CYAN);
    } else {
        canvas_fill_ellipse(c, 168, 100, 6, 8, PV_CLOAK_SHADOW);
        canvas_fill_ellipse(c, 196, 100, 6, 8, PV_CLOAK_SHADOW);
    }

    /* ---- hand nubs (small rounded shapes on either side) ---- */
    canvas_fill_circle(c, 105 + arm_offset, 195, 12, PV_CLOAK_GREY);
    canvas_fill_circle(c, 259 - arm_offset, 195, 12, PV_CLOAK_GREY);
    canvas_fill_circle(c, 105 + arm_offset, 195, 7, PV_CLOAK_HI);
    canvas_fill_circle(c, 259 - arm_offset, 195, 7, PV_CLOAK_HI);

    /* ---- legs (two stubby pillars from legs_y down) ---- */
    canvas_fill_rect_rounded(c, 135, legs_y, 38, 50, 8, PV_CLOAK_GREY);
    canvas_fill_rect_rounded(c, 191, legs_y, 38, 50, 8, PV_CLOAK_GREY);
    /* feet (void black) */
    canvas_fill_rect_rounded(c, 133, legs_y + 38, 42, 14, 6, PV_VOID_BLACK);
    canvas_fill_rect_rounded(c, 189, legs_y + 38, 42, 14, 6, PV_VOID_BLACK);

    /* ---- ambient void wisp curling off the back of the hood ---- */
    canvas_draw_line(c, 125, 85, 100, 65, 5, 0xA02A0A3A);
    canvas_draw_line(c, 100, 65, 115, 45, 4, 0x802A0A3A);
    canvas_fill_radial_glow(c, 115, 45, 8, PV_VOID_BRIGHT, 100);
}

/* ---- idle: standing, slight breathing (legs straight) ---- */
static inline void player_idle_draw(Canvas* c) {
    canvas_clear(c, 0);
    player_draw_body(c, /*legs_y=*/240, /*arm_offset=*/0, /*glow_eyes=*/true);
}

/* ---- walk1: legs splayed (left forward, right back) ---- */
static inline void player_walk1_draw(Canvas* c) {
    canvas_clear(c, 0);
    /* shift the right leg forward, left leg back */
    player_draw_body(c, /*legs_y=*/240, /*arm_offset=*/4, /*glow_eyes=*/true);
    /* overlay: move right leg forward */
    canvas_fill_rect_rounded(c, 130, 240, 40, 50, 8, PV_CLOAK_GREY);
    canvas_fill_rect_rounded(c, 194, 240, 40, 50, 8, PV_CLOAK_GREY);
    /* re-draw feet offset */
    canvas_fill_rect_rounded(c, 110, 280, 44, 14, 6, PV_VOID_BLACK);  /* left foot back */
    canvas_fill_rect_rounded(c, 210, 280, 44, 14, 6, PV_VOID_BLACK);  /* right foot fwd */
}

/* ---- walk2: legs splayed opposite ---- */
static inline void player_walk2_draw(Canvas* c) {
    canvas_clear(c, 0);
    player_draw_body(c, /*legs_y=*/240, /*arm_offset=*/-4, /*glow_eyes=*/true);
    canvas_fill_rect_rounded(c, 130, 240, 40, 50, 8, PV_CLOAK_GREY);
    canvas_fill_rect_rounded(c, 194, 240, 40, 50, 8, PV_CLOAK_GREY);
    canvas_fill_rect_rounded(c, 116, 280, 44, 14, 6, PV_VOID_BLACK);
    canvas_fill_rect_rounded(c, 204, 280, 44, 14, 6, PV_VOID_BLACK);
}

/* ---- jump: legs tucked up, arms out ---- */
static inline void player_jump_draw(Canvas* c) {
    canvas_clear(c, 0);
    /* body shifted up slightly, legs tucked */
    player_draw_body(c, /*legs_y=*/250, /*arm_offset=*/10, /*glow_eyes=*/true);
    /* overlay: tucked legs (shorter, higher) */
    canvas_fill_rect_rounded(c, 134, 250, 36, 30, 8, PV_CLOAK_GREY);
    canvas_fill_rect_rounded(c, 194, 250, 36, 30, 8, PV_CLOAK_GREY);
    canvas_fill_rect_rounded(c, 132, 275, 40, 12, 6, PV_VOID_BLACK);
    canvas_fill_rect_rounded(c, 192, 275, 40, 12, 6, PV_VOID_BLACK);
}

/* ---- attack: voidscrew extended horizontally ---- */
static inline void player_attack_draw(Canvas* c) {
    canvas_clear(c, 0);
    player_draw_body(c, /*legs_y=*/240, /*arm_offset=*/0, /*glow_eyes=*/true);
    /* voidscrew blade extending to the right */
    canvas_fill_rect_rounded(c, 264, 190, 80, 12, 4, PV_CLOAK_HI);     /* hilt */
    canvas_fill_triangle(c, 340, 180, 340, 212, 364, 196, PV_VOIDSCREW_GLOW); /* blade tip */
    /* blade glow trail */
    canvas_fill_radial_glow(c, 320, 196, 20, PV_VOIDSCREW_GLOW, 120);
    canvas_fill_radial_glow(c, 350, 196, 16, PV_VOID_BRIGHT, 100);
}

/* ---- pogo: voidscrew pointing down, legs up ---- */
static inline void player_pogo_draw(Canvas* c) {
    canvas_clear(c, 0);
    player_draw_body(c, /*legs_y=*/250, /*arm_offset=*/0, /*glow_eyes=*/true);
    canvas_fill_rect_rounded(c, 140, 250, 30, 24, 8, PV_CLOAK_GREY);
    canvas_fill_rect_rounded(c, 194, 250, 30, 24, 8, PV_CLOAK_GREY);
    canvas_fill_rect_rounded(c, 170, 290, 24, 50, 4, PV_CLOAK_HI);
    canvas_fill_triangle(c, 160, 340, 194, 340, 182, 364, PV_VOIDSCREW_GLOW);
    canvas_fill_radial_glow(c, 182, 348, 16, PV_VOIDSCREW_GLOW, 140);
    canvas_fill_radial_glow(c, 182, 356, 12, PV_VOID_BRIGHT, 120);
}

#endif /* ART_PLAYER_SPRITE_H */
