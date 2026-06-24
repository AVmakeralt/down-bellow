#ifndef ART_TOWN_TILES_H
#define ART_TOWN_TILES_H

#include "../src/canvas.h"

/*
 * Town tiles for the Ash-Foot Town biome. Kept at 32x32 grid strings (not
 * procedural 364x364) because:
 *   - tiles are background texture, not focal sprites
 *   - the pixel-art tile aesthetic pairs well with painted character sprites
 *     (think Owlboy / Iconoclasts)
 *   - 364x364 tiles would make the world look over-rendered next to simple
 *     platforming geometry
 *
 * If you want painted tiles too, swap these for procedural functions like
 * the player/crawler sprites.
 */

#define TILE_W 32
#define TILE_H 32

/* --- stone (mortar + brick pattern) ------------------------------- */
static const char* tile_stone[TILE_H] = {
    "99999999999999999999999999999999",
    "9aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa9",
    "9aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa9",
    "9aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa9",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "99999999999999999999999999999999",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "99999999999999999999999999999999",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "99999999999999999999999999999999",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "99999999999999999999999999999999",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
};

/* --- moss (mossy top, stone below) ------------------------------- */
static const char* tile_moss[TILE_H] = {
    "cccccccccccccccccccccccccccccccc",
    "cccccccccccccccccccccccccccccccc",
    "cccaaccccaaaccccaaaccccaaaccccaa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "99999999999999999999999999999999",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "99999999999999999999999999999999",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "99999999999999999999999999999999",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",
    "99999999999999999999999999999999",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",
};

/* --- ground (dark earth) ---------------------------------------- */
static const char* tile_ground[TILE_H] = {
    "99999999999999999999999999999999",
    "94444444944444994444494444449944",
    "94444449944444994444494444449944",
    "94444999944444994444494444444994",
    "99444444994444494444494444444994",
    "94444444444444444444494444449944",
    "94444444944444444444494444449444",
    "94444444994444444444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
    "94444444994444494444494444449944",
};

/* --- bgwall (faint background, parallax) ------------------------ */
static const char* tile_bgwall[TILE_H] = {
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
    "44444444444444444444444444444444",
};

#endif /* ART_TOWN_TILES_H */
