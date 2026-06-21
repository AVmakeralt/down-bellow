#ifndef ART_TOWN_TILES_H
#define ART_TOWN_TILES_H

/*
 * Town tiles for the Ash-Foot Town biome. 32x32 each, hand-authored.
 *
 * Every row is exactly 32 chars. Brick rows repeat an 8-char pattern
 * 4 times (8*4 = 32) so they tile seamlessly horizontally.
 *
 * Palette:
 *   4 = dirt / cloak shadow
 *   9 = stone dark (mortar lines)
 *   a = stone
 *   c = moss
 *
 * Tiles: stone, moss, ground, bgwall
 *
 * Legend for level files (see levels txt files):
 *   '#' = stone      (solid)
 *   'M' = moss       (solid, top of walls)
 *   '=' = ground     (solid, floor)
 *   '.' = bgwall     (non-solid background, parallax)
 *   ' ' = empty      (non-solid, no draw)
 */

#define TILE_W 32
#define TILE_H 32

/* --- stone (mortar + brick pattern) ------------------------------- */
static const char* tile_stone[TILE_H] = {
    "99999999999999999999999999999999",  /*  0 mortar */
    "9aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa9",  /*  1 */
    "9aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa9",  /*  2 */
    "9aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa9",  /*  3 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /*  4 brick row A */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /*  5 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /*  6 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /*  7 */
    "99999999999999999999999999999999",  /*  8 mortar */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /*  9 brick row B (offset) */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 10 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 11 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 12 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 13 */
    "99999999999999999999999999999999",  /* 14 mortar */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 15 brick row A */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 16 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 17 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 18 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 19 */
    "99999999999999999999999999999999",  /* 20 mortar */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 21 brick row B */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 22 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 23 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 24 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 25 */
    "99999999999999999999999999999999",  /* 26 mortar */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 27 brick row A */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 28 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 29 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 30 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 31 */
};

/* --- moss (mossy top, stone below; for wall caps) ---------------- */
static const char* tile_moss[TILE_H] = {
    "cccccccccccccccccccccccccccccccc",  /*  0 moss top */
    "cccccccccccccccccccccccccccccccc",  /*  1 */
    "cccaaccccaaaccccaaaccccaaaccccaa",  /*  2 mossy edge */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /*  3 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /*  4 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /*  5 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /*  6 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /*  7 */
    "99999999999999999999999999999999",  /*  8 mortar */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /*  9 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 10 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 11 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 12 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 13 */
    "99999999999999999999999999999999",  /* 14 mortar */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 15 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 16 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 17 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 18 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 19 */
    "99999999999999999999999999999999",  /* 20 mortar */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 21 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 22 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 23 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 24 */
    "9aaaa9aa9aaaa9aa9aaaa9aa9aaaa9aa",  /* 25 */
    "99999999999999999999999999999999",  /* 26 mortar */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 27 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 28 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 29 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 30 */
    "9aa9aaaa9aa9aaaa9aa9aaaa9aa9aaaa",  /* 31 */
};

/* --- ground (dark earth with pebbles) ---------------------------- */
static const char* tile_ground[TILE_H] = {
    "99999999999999999999999999999999",  /*  0 top crust */
    "94444444944444994444494444449944",  /*  1 */
    "94444449944444994444494444449944",  /*  2 */
    "94444999944444994444494444444994",  /*  3 */
    "99444444994444494444494444444994",  /*  4 */
    "94444444444444444444494444449944",  /*  5 */
    "94444444944444444444494444449444",  /*  6 */
    "94444444994444444444494444449944",  /*  7 */
    "94444444994444494444494444449944",  /*  8 */
    "94444444994444494444494444449944",  /*  9 */
    "94444444994444494444494444449944",  /* 10 */
    "94444444994444494444494444449944",  /* 11 */
    "94444444994444494444494444449944",  /* 12 */
    "94444444994444494444494444449944",  /* 13 */
    "94444444994444494444494444449944",  /* 14 */
    "94444444994444494444494444449944",  /* 15 */
    "94444444994444494444494444449944",  /* 16 */
    "94444444994444494444494444449944",  /* 17 */
    "94444444994444494444494444449944",  /* 18 */
    "94444444994444494444494444449944",  /* 19 */
    "94444444994444494444494444449944",  /* 20 */
    "94444444994444494444494444449944",  /* 21 */
    "94444444994444494444494444449944",  /* 22 */
    "94444444994444494444494444449944",  /* 23 */
    "94444444994444494444494444449944",  /* 24 */
    "94444444994444494444494444449944",  /* 25 */
    "94444444994444494444494444449944",  /* 26 */
    "94444444994444494444494444449944",  /* 27 */
    "94444444994444494444494444449944",  /* 28 */
    "94444444994444494444494444449944",  /* 29 */
    "94444444994444494444494444449944",  /* 30 */
    "94444444994444494444494444449944",  /* 31 */
};

/* --- bgwall (faint background, drawn behind parallax) ----------- *
 * Single flat value; used as a wall texture for distant buildings.
 */
static const char* tile_bgwall[TILE_H] = {
    "44444444444444444444444444444444",  /*  0 */
    "44444444444444444444444444444444",  /*  1 */
    "44444444444444444444444444444444",  /*  2 */
    "44444444444444444444444444444444",  /*  3 */
    "44444444444444444444444444444444",  /*  4 */
    "44444444444444444444444444444444",  /*  5 */
    "44444444444444444444444444444444",  /*  6 */
    "44444444444444444444444444444444",  /*  7 */
    "44444444444444444444444444444444",  /*  8 */
    "44444444444444444444444444444444",  /*  9 */
    "44444444444444444444444444444444",  /* 10 */
    "44444444444444444444444444444444",  /* 11 */
    "44444444444444444444444444444444",  /* 12 */
    "44444444444444444444444444444444",  /* 13 */
    "44444444444444444444444444444444",  /* 14 */
    "44444444444444444444444444444444",  /* 15 */
    "44444444444444444444444444444444",  /* 16 */
    "44444444444444444444444444444444",  /* 17 */
    "44444444444444444444444444444444",  /* 18 */
    "44444444444444444444444444444444",  /* 19 */
    "44444444444444444444444444444444",  /* 20 */
    "44444444444444444444444444444444",  /* 21 */
    "44444444444444444444444444444444",  /* 22 */
    "44444444444444444444444444444444",  /* 23 */
    "44444444444444444444444444444444",  /* 24 */
    "44444444444444444444444444444444",  /* 25 */
    "44444444444444444444444444444444",  /* 26 */
    "44444444444444444444444444444444",  /* 27 */
    "44444444444444444444444444444444",  /* 28 */
    "44444444444444444444444444444444",  /* 29 */
    "44444444444444444444444444444444",  /* 30 */
    "44444444444444444444444444444444",  /* 31 */
};

#endif /* ART_TOWN_TILES_H */
