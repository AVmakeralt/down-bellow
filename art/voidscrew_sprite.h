#ifndef ART_VOIDSCREW_SPRITE_H
#define ART_VOIDSCREW_SPRITE_H

/*
 * Voidscrew - the player's small blade. 16x16 grid sprite, hand-authored.
 * Used in HUD / inventory panels; the in-world slash is rendered via the
 * player's attack/pogo frames.
 *
 * Palette:
 *   1 = void black
 *   2 = cloak grey (hilt wrap)
 *   3 = cloak highlight (crossguard)
 *   f = voidscrew glow
 */

#define VOIDSCREW_W 16
#define VOIDSCREW_H 16

static const char* voidscrew_idle[VOIDSCREW_H] = {
    "................",  /*  0 */
    "......11........",  /*  1 hilt top */
    ".....1221.......",  /*  2 */
    ".....1221.......",  /*  3 hilt wrap */
    ".....1221.......",  /*  4 */
    "......11........",  /*  5 crossguard base */
    ".....1331.......",  /*  6 crossguard */
    "....1ffff1......",  /*  7 blade glow start */
    "....1ffff1......",  /*  8 */
    ".....ffff.......",  /*  9 */
    ".....ffff.......",  /* 10 */
    "......ff........",  /* 11 */
    "......ff........",  /* 12 */
    ".......f........",  /* 13 tip */
    "................",  /* 14 */
    "................",  /* 15 */
};

#endif /* ART_VOIDSCREW_SPRITE_H */
