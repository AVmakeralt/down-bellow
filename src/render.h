#ifndef TV_RENDER_H
#define TV_RENDER_H

#include <SDL2/SDL.h>
#include "types.h"

/*
 * Renderer.
 *
 * Grid sprites are baked once into a single SDL_Texture atlas at startup.
 * Each sprite is identified by a small integer ID; draw calls just blit a
 * sub-rect. This keeps per-frame draw cost minimal even on software
 * renderers.
 */

typedef int SpriteID;

#define SPRITE_NONE        (-1)
#define SPRITE_PLAYER_IDLE    0
#define SPRITE_PLAYER_WALK1   1
#define SPRITE_PLAYER_WALK2   2
#define SPRITE_PLAYER_JUMP    3
#define SPRITE_PLAYER_ATTACK  4
#define SPRITE_PLAYER_POGO    5
#define SPRITE_VOIDSCREW      6
#define SPRITE_CRAWLER_IDLE   7
#define SPRITE_CRAWLER_WALK1  8
#define SPRITE_CRAWLER_WALK2  9
#define SPRITE_CRAWLER_ATTACK 10
#define SPRITE_CRAWLER_HURT   11
#define SPRITE_TILE_STONE     12
#define SPRITE_TILE_MOSS      13
#define SPRITE_TILE_GROUND    14
#define SPRITE_TILE_BGWALL    15
#define SPRITE_COUNT          16

typedef struct {
    SDL_Texture* texture;
    int          w, h;        /* full atlas size */
    SDL_Rect     rects[SPRITE_COUNT];
} SpriteAtlas;

typedef struct {
    SDL_Window*   window;
    SDL_Renderer* rend;
    SpriteAtlas   atlas;
    int           window_w;
    int           window_h;
    int           logical_w;       /* internal render resolution (e.g. 480) */
    int           logical_h;       /* (e.g. 270) — sprite stage space        */
    bool          debug;

    /* Pre-baked post textures (built once in renderer_init) */
    SDL_Texture*  void_glow;       /* radial purple glow, additive blend  */
    SDL_Texture*  vignette;        /* dark-edge overlay, multiply blend   */
    SDL_Texture*  grain;           /* 64x64 noise tile, screen blend       */

    /* Per-sprite textures for procedural 364x364 sprites.
     * Tiles (SPRITE_TILE_*) stay in the atlas; sprites get their own
     * texture so we can use LINEAR scaling for painted look. */
    SDL_Texture*  sprite_tex[SPRITE_COUNT];
    int           sprite_w[SPRITE_COUNT];   /* source width  (364 for proc, 32 for tiles) */
    int           sprite_h[SPRITE_COUNT];   /* source height */
} Renderer;

bool renderer_init(Renderer* r, int window_w, int window_h);
void renderer_free(Renderer* r);

bool atlas_build(SpriteAtlas* atlas, SDL_Renderer* rend);
void atlas_free(SpriteAtlas* atlas);

void renderer_clear(Renderer* r, uint32_t color_abgr);
void renderer_present(Renderer* r);

/* Draw a sprite at world-space (x, y), flipped horizontally if flip!=0. */
void draw_sprite(Renderer* r, SpriteID id, float x, float y, int flip);
/* Draw a sprite at screen-space (sx, sy) at native source size. */
void draw_sprite_screen(Renderer* r, SpriteID id, int sx, int sy, int flip);
/* Draw a sprite at screen-space (sx, sy) scaled to (dw, dh). Used for
 * 364x364 procedural sprites displayed at ~96px. */
void draw_sprite_screen_scaled(Renderer* r, SpriteID id, int sx, int sy,
                               int dw, int dh, int flip);

/* Draw a colored rect in screen-space (for debug hitboxes, etc.) */
void draw_rect_screen(Renderer* r, IRect rc, uint32_t color_abgr, int filled);

/* Draw the pre-baked void glow at (sx,sy) with given radius (screen-space).
 * Uses additive blending so it lights up whatever is underneath. */
void draw_void_glow(Renderer* r, int sx, int sy, int radius_px, uint8_t alpha);

/* Draw the pre-baked vignette overlay (fullscreen, multiply blend). */
void draw_vignette(Renderer* r);

/* Draw the pre-baked film grain (fullscreen, screen blend, animated by tick). */
void draw_grain(Renderer* r, int tick);

#endif /* TV_RENDER_H */
