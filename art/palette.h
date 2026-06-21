#ifndef ART_PALETTE_H
#define ART_PALETTE_H

#include <stdint.h>

/*
 * The Voided - shared 16-color palette.
 *
 * Index 0 is TRANSPARENT (never drawn). Indices 1..15 are packed ABGR
 * (matching SDL_PIXELFORMAT_ABGR8888) so we can copy straight into a
 * software surface pixel buffer.
 *
 * Sprite character mapping (used in every art (dot h) file):
 *
 *     '.'  -> 0   transparent
 *     '1'  -> 1   void black        (cloak outline)
 *     '2'  -> 2   cloak grey        (main body)
 *     '3'  -> 3   cloak highlight   (rim light)
 *     '4'  -> 4   cloak shadow / dirt
 *     '5'  -> 5   eye glow white
 *     '6'  -> 6   eye glow cyan
 *     '7'  -> 7   void purple
 *     '8'  -> 8   void purple bright
 *     '9'  -> 9   stone dark
 *     'a'  -> 10  stone
 *     'b'  -> 11  stone light
 *     'c'  -> 12  moss
 *     'd'  -> 13  fog
 *     'e'  -> 14  blood red
 *     'f'  -> 15  voidscrew glow
 *
 * Tweak any color below to retune the look of the whole game.
 */
static const uint32_t PALETTE[16] = {
    0x00000000,  /* 0  transparent                          */
    0xFF12060A,  /* 1  void black      (deeper, bluer)      */
    0xFF332B40,  /* 2  cloak grey      (cool purple-grey)   */
    0xFF55406A,  /* 3  cloak highlight (purple rim light)  */
    0xFF1F0E26,  /* 4  cloak shadow / dirt (void-tinted)    */
    0xFFEAE0F0,  /* 5  eye glow white  (cool, not warm)     */
    0xFF88E0FF,  /* 6  eye glow CYAN   (actual cyan)        */
    0xFF2A0A3A,  /* 7  void purple     (darker, ambience)   */
    0xFFC030FF,  /* 8  void purple BRIGHT (signature)       */
    0xFF181018,  /* 9  stone dark                            */
    0xFF332030,  /* a  stone (purple-tinted, not gray)       */
    0xFF553D55,  /* b  stone light                           */
    0xFF1F3A2A,  /* c  moss (deeper)                         */
    0xFFB0A0C0,  /* d  fog (cool, not warm)                  */
    0xFFFF2A4A,  /* e  blood red (punchier)                  */
    0xFFFFFFEE,  /* f  voidscrew glow (true white-violet)    */
};

/* Convert a sprite character into a palette index (0..15). */
static inline uint8_t palette_index_from_char(char c) {
    if (c == '.' || c == ' ') return 0;
    if (c >= '1' && c <= '9') return (uint8_t)(c - '1' + 1);
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A' + 10);
    return 0;
}

#endif /* ART_PALETTE_H */
