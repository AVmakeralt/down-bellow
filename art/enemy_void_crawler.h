#ifndef ART_ENEMY_VOID_CRAWLER_H
#define ART_ENEMY_VOID_CRAWLER_H

#include "../src/canvas.h"

/*
 * ENEMY: Void Crawler — procedural 364x364 sprite.
 *
 * Low-slung corrupted shade. Body is a wide purple ellipse, one big
 * pulsing eye in the center, blood-red mouth underneath, four legs
 * splaying out to the sides. Claws extend forward when attacking.
 *
 * Palette:
 *   VOID_BLACK     0xFF12060A
 *   VOID_PURPLE    0xFF2A0A3A   (body shadow)
 *   VOID_BRIGHT    0xFFC030FF   (eye / claw glow)
 *   BLOOD_RED      0xFFFF2A4A   (mouth)
 *   CLOAK_SHADOW   0xFF1F0E26   (leg tips)
 *
 * Frames: idle, walk1, walk2, attack, hurt
 * The eye pulses at runtime via the crawler_draw vtable (overdraw).
 */

#define CV_VOID_BLACK     0xFF12060A
#define CV_VOID_PURPLE    0xFF2A0A3A
#define CV_VOID_BRIGHT    0xFFC030FF
#define CV_BLOOD_RED      0xFFFF2A4A
#define CV_LEG_TIP        0xFF1F0E26

/* shared body helper.
 * leg_phase: 0 = neutral, +1 = legs shifted right, -1 = shifted left */
static inline void crawler_draw_body(Canvas* c, int leg_phase, bool glow_eye) {
    int cx = 182, cy = 200;

    /* ---- shadow under body ---- */
    canvas_fill_ellipse(c, cx, 290, 90, 16, 0x50000000);

    /* ---- legs (4 splaying out, drawn before body so body overlaps tops) ---- */
    /* front-left, back-left, front-right, back-right */
    int leg_offsets[4][2] = {
        { -50,  40 + leg_phase * 6 },  /* front-left  */
        { -70,  20 - leg_phase * 4 },  /* back-left   */
        {  50,  40 - leg_phase * 6 },  /* front-right */
        {  70,  20 + leg_phase * 4 },  /* back-right  */
    };
    for (int i = 0; i < 4; i++) {
        int lx = cx + leg_offsets[i][0];
        int ly = cy + 30 + leg_offsets[i][1];
        canvas_draw_line(c, cx, cy + 10, lx, ly, 8, CV_VOID_PURPLE);
        canvas_fill_circle(c, lx, ly, 6, CV_LEG_TIP);
    }

    /* ---- body (wide ellipse, vertical gradient) ---- */
    canvas_fill_ellipse(c, cx, cy, 95, 55, CV_VOID_PURPLE);
    /* top highlight */
    canvas_fill_ellipse(c, cx, cy - 15, 80, 30, 0xFF4A1060);
    /* bottom shadow */
    canvas_fill_ellipse(c, cx, cy + 20, 85, 25, 0xFF1A051F);

    /* ---- eye (single large glowing eye in center) ---- */
    if (glow_eye) {
        /* outer glow */
        canvas_fill_radial_glow(c, cx, cy - 5, 40, CV_VOID_BRIGHT, 160);
        /* eye socket */
        canvas_fill_ellipse(c, cx, cy - 5, 24, 18, CV_VOID_BLACK);
        /* iris (bright purple) */
        canvas_fill_ellipse(c, cx, cy - 5, 18, 14, CV_VOID_BRIGHT);
        /* pupil (void black) */
        canvas_fill_ellipse(c, cx, cy - 5, 8, 10, CV_VOID_BLACK);
        /* catchlight */
        canvas_fill_circle(c, cx - 5, cy - 10, 3, 0xFFFFFFFF);
    } else {
        canvas_fill_ellipse(c, cx, cy - 5, 20, 14, CV_LEG_TIP);
    }

    /* ---- mouth (blood-red gash under eye) ---- */
    canvas_fill_rect_rounded(c, cx - 28, cy + 22, 56, 10, 4, CV_BLOOD_RED);
    /* teeth (small dark notches) */
    canvas_fill_rect(c, cx - 20, cy + 24, 4, 6, CV_VOID_BLACK);
    canvas_fill_rect(c, cx - 8,  cy + 24, 4, 6, CV_VOID_BLACK);
    canvas_fill_rect(c, cx + 4,  cy + 24, 4, 6, CV_VOID_BLACK);
    canvas_fill_rect(c, cx + 16, cy + 24, 4, 6, CV_VOID_BLACK);
}

/* ---- idle: legs neutral, eye scanning ---- */
static inline void crawler_idle_draw(Canvas* c) {
    canvas_clear(c, 0);
    crawler_draw_body(c, /*leg_phase=*/0, /*glow_eye=*/true);
}

/* ---- walk1: legs shifted right (lunge forward) ---- */
static inline void crawler_walk1_draw(Canvas* c) {
    canvas_clear(c, 0);
    crawler_draw_body(c, /*leg_phase=*/1, /*glow_eye=*/true);
}

/* ---- walk2: legs shifted left ---- */
static inline void crawler_walk2_draw(Canvas* c) {
    canvas_clear(c, 0);
    crawler_draw_body(c, /*leg_phase=*/-1, /*glow_eye=*/true);
}

/* ---- attack: reared up, claws forward glowing ---- */
static inline void crawler_attack_draw(Canvas* c) {
    canvas_clear(c, 0);
    /* body shifted up slightly (reared) */
    crawler_draw_body(c, /*leg_phase=*/0, /*glow_eye=*/true);
    /* claws: two glowing forward extensions on the right side */
    canvas_draw_line(c, 270, 180, 320, 170, 6, CV_VOID_BRIGHT);
    canvas_draw_line(c, 270, 210, 325, 210, 6, CV_VOID_BRIGHT);
    /* claw tips */
    canvas_fill_triangle(c, 318, 165, 328, 170, 322, 178, 0xFFFFFFEE);
    canvas_fill_triangle(c, 323, 205, 333, 210, 327, 218, 0xFFFFFFEE);
    /* claw glow */
    canvas_fill_radial_glow(c, 325, 190, 22, CV_VOID_BRIGHT, 140);
    /* wider mouth (snarling) */
    canvas_fill_rect_rounded(c, 154, 222, 56, 14, 4, CV_BLOOD_RED);
}

/* ---- hurt: eye flickers red, body flattened ---- */
static inline void crawler_hurt_draw(Canvas* c) {
    canvas_clear(c, 0);
    crawler_draw_body(c, /*leg_phase=*/0, /*glow_eye=*/false);
    /* red eye flicker (overdraw) */
    canvas_fill_ellipse(c, 182, 195, 18, 14, CV_BLOOD_RED);
    /* body tint red */
    canvas_fill_ellipse(c, 182, 200, 95, 55, 0x40FF2A4A);
}

#endif /* ART_ENEMY_VOID_CRAWLER_H */
