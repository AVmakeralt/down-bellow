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
    0xFF140A0A,  /* 1  void black      (cloak outline)      */
    0xFF4E3A3A,  /* 2  cloak grey      (main body)          */
    0xFF725A5A,  /* 3  cloak highlight (rim light)          */
    0xFF2E2020,  /* 4  cloak shadow / dirt                  */
    0xFFF0E0E0,  /* 5  eye glow white                       */
    0xFFFFCC88,  /* 6  eye glow cyan                        */
    0xFF54143A,  /* 7  void purple                          */
    0xFFA42D7A,  /* 8  void purple bright                   */
    0xFF30262A,  /* 9  stone dark                           */
    0xFF54464A,  /* a  stone                                */
    0xFF74666A,  /* b  stone light                          */
    0xFF2A4A3A,  /* c  moss                                 */
    0xFFB4A8A8,  /* d  fog                                  */
    0xFF2020A8,  /* e  blood red                            */
    0xFFFFB8C8,  /* f  voidscrew glow                       */
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
