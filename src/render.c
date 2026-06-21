#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../art/palette.h"
#include "../art/player_sprite.h"
#include "../art/voidscrew_sprite.h"
#include "../art/enemy_void_crawler.h"
#include "../art/town_tiles.h"

/* forward decls for pre-bake helpers (defined below) */
static SDL_Texture* bake_radial_glow(SDL_Renderer* rend, int radius,
                                     uint32_t rgb_abgr_no_alpha, uint8_t max_alpha);
static SDL_Texture* bake_vignette(SDL_Renderer* rend, int w, int h);
static SDL_Texture* bake_grain(SDL_Renderer* rend, int side);

/* ---- atlas: each sprite baked into a horizontal strip --------------- *
 * Layout (matches SPRITE_* enum in render.h):
 *   0..5  player frames  (32x32 each)
 *   6     voidscrew      (16x16)
 *   7..11 crawler frames (32x32 each)
 *   12..15 tiles          (32x32 each)
 */
typedef struct {
    const char** rows;
    int w, h;
} SpriteDef;

static SpriteDef sprite_defs[SPRITE_COUNT] = {
    [SPRITE_PLAYER_IDLE]    = { (const char**)player_idle,    PLAYER_W,  PLAYER_H  },
    [SPRITE_PLAYER_WALK1]   = { (const char**)player_walk1,   PLAYER_W,  PLAYER_H  },
    [SPRITE_PLAYER_WALK2]   = { (const char**)player_walk2,   PLAYER_W,  PLAYER_H  },
    [SPRITE_PLAYER_JUMP]    = { (const char**)player_jump,    PLAYER_W,  PLAYER_H  },
    [SPRITE_PLAYER_ATTACK]  = { (const char**)player_attack,  PLAYER_W,  PLAYER_H  },
    [SPRITE_PLAYER_POGO]    = { (const char**)player_pogo,    PLAYER_W,  PLAYER_H  },
    [SPRITE_VOIDSCREW]      = { (const char**)voidscrew_idle, VOIDSCREW_W, VOIDSCREW_H },
    [SPRITE_CRAWLER_IDLE]   = { (const char**)crawler_idle,   CRAWLER_W, CRAWLER_H },
    [SPRITE_CRAWLER_WALK1]  = { (const char**)crawler_walk1,  CRAWLER_W, CRAWLER_H },
    [SPRITE_CRAWLER_WALK2]  = { (const char**)crawler_walk2,  CRAWLER_W, CRAWLER_H },
    [SPRITE_CRAWLER_ATTACK] = { (const char**)crawler_attack, CRAWLER_W, CRAWLER_H },
    [SPRITE_CRAWLER_HURT]   = { (const char**)crawler_hurt,   CRAWLER_W, CRAWLER_H },
    [SPRITE_TILE_STONE]     = { (const char**)tile_stone,     TILE_W,    TILE_H    },
    [SPRITE_TILE_MOSS]      = { (const char**)tile_moss,      TILE_W,    TILE_H    },
    [SPRITE_TILE_GROUND]    = { (const char**)tile_ground,    TILE_W,    TILE_H    },
    [SPRITE_TILE_BGWALL]    = { (const char**)tile_bgwall,    TILE_W,    TILE_H    },
};

bool renderer_init(Renderer* r, int window_w, int window_h) {
    memset(r, 0, sizeof(*r));
    r->window_w = window_w;
    r->window_h = window_h;

    /* Internal render resolution: 480x270 (16:9, 1/8 the pixel count of
     * 1280x720). SDL_RenderSetLogicalSize handles the letterbox + nearest-
     * neighbor upscale for us. This makes our 32x32 sprites ~11% of the
     * screen width instead of 2.5% — every hand-drawn detail becomes
     * visible and we get the chunky pixel-art aesthetic for free.
     *
     * Tunable: bump to 640x360 if you want it slightly less chunky. */
    r->logical_w = 480;
    r->logical_h = 270;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    r->window = SDL_CreateWindow(
        "The Voided",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_w, window_h,
        SDL_WINDOW_SHOWN);
    if (!r->window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return false;
    }
    r->rend = SDL_CreateRenderer(r->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!r->rend) {
        r->rend = SDL_CreateRenderer(r->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!r->rend) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return false;
    }
    /* THE magic call. All subsequent draw calls operate in 480x270 space
     * and SDL upscales to the window with nearest-neighbor. */
    SDL_RenderSetLogicalSize(r->rend, r->logical_w, r->logical_h);
    /* Force nearest-neighbor (pixel-perfect) instead of smooth (blurry). */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_SetRenderDrawBlendMode(r->rend, SDL_BLENDMODE_BLEND);

    if (!atlas_build(&r->atlas, r->rend)) {
        fprintf(stderr, "atlas_build failed\n");
        return false;
    }

    /* Pre-bake post textures */
    r->void_glow = bake_radial_glow(r->rend, 64, 0xC030FF, 255);
    r->vignette  = bake_vignette(r->rend, r->logical_w, r->logical_h);
    r->grain     = bake_grain(r->rend, 64);

    return true;
}

void renderer_free(Renderer* r) {
    atlas_free(&r->atlas);
    if (r->void_glow) SDL_DestroyTexture(r->void_glow);
    if (r->vignette)  SDL_DestroyTexture(r->vignette);
    if (r->grain)     SDL_DestroyTexture(r->grain);
    if (r->rend)   SDL_DestroyRenderer(r->rend);
    if (r->window) SDL_DestroyWindow(r->window);
    SDL_Quit();
}

/* ---- pre-bake: radial gradient glow texture ---------------------- *
 * Used for the player's void aura (additive blend).
 */
static SDL_Texture* bake_radial_glow(SDL_Renderer* rend, int radius,
                              uint32_t rgb_abgr_no_alpha, uint8_t max_alpha) {
    int side = radius * 2;
    Uint32 rmask = 0x000000FF, gmask = 0x0000FF00,
           bmask = 0x00FF0000, amask = 0xFF000000;
    SDL_Surface* s = SDL_CreateRGBSurface(0, side, side, 32,
                                          rmask, gmask, bmask, amask);
    if (!s) return NULL;
    SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_NONE);
    Uint32* px = (Uint32*)s->pixels;
    for (int y = 0; y < side; y++) {
        for (int x = 0; x < side; x++) {
            float dx = (float)(x - radius);
            float dy = (float)(y - radius);
            float d = sqrtf(dx*dx + dy*dy) / (float)radius;
            float a = 1.0f - d;
            if (a < 0) a = 0;
            a = a*a*a*a;   /* ease-out quartic */
            Uint8 alpha = (Uint8)(a * (float)max_alpha);
            /* rgb from rgb_abgr_no_alpha (which has alpha=00) */
            Uint32 rgb = rgb_abgr_no_alpha & 0x00FFFFFF;
            px[y * side + x] = rgb | ((Uint32)alpha << 24);
        }
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, s);
    SDL_FreeSurface(s);
    if (tex) {
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
        SDL_SetTextureScaleMode(tex, SDL_ScaleModeLinear);
    }
    return tex;
}

/* ---- pre-bake: vignette (dark edges) ----------------------------- *
 * Multiply-blended over the final scene. Concentrates the eye on the
 * center, hides letterbox edges.
 */
static SDL_Texture* bake_vignette(SDL_Renderer* rend, int w, int h) {
    Uint32 rmask = 0x000000FF, gmask = 0x0000FF00,
           bmask = 0x00FF0000, amask = 0xFF000000;
    SDL_Surface* s = SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);
    if (!s) return NULL;
    SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_NONE);
    Uint32* px = (Uint32*)s->pixels;
    float cx = w * 0.5f, cy = h * 0.5f;
    float max_d = sqrtf(cx*cx + cy*cy);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float dx = x - cx, dy = y - cy;
            float d = sqrtf(dx*dx + dy*dy) / max_d;   /* 0 center, 1 corner */
            /* dark = 0 at center, ~0.65 at edges */
            float dark = d * d * 0.85f;
            if (dark > 0.85f) dark = 0.85f;
            Uint8 a = (Uint8)(dark * 255.0f);
            px[y * w + x] = 0x00000000 | ((Uint32)a << 24);
        }
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, s);
    SDL_FreeSurface(s);
    if (tex) SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

/* ---- pre-bake: film grain noise tile ----------------------------- *
 * 64x64 random grey noise. Tiled across screen at low alpha with
 * screen blend. Hides color banding, adds analog texture.
 */
static SDL_Texture* bake_grain(SDL_Renderer* rend, int side) {
    Uint32 rmask = 0x000000FF, gmask = 0x0000FF00,
           bmask = 0x00FF0000, amask = 0xFF000000;
    SDL_Surface* s = SDL_CreateRGBSurface(0, side, side, 32, rmask, gmask, bmask, amask);
    if (!s) return NULL;
    SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_NONE);
    Uint32* px = (Uint32*)s->pixels;
    for (int y = 0; y < side; y++) {
        for (int x = 0; x < side; x++) {
            /* random grey value 180-255 (light noise on dark scenes) */
            Uint8 v = 180 + (rand() % 76);
            Uint8 a = 18;   /* ~7% opacity */
            px[y * side + x] = ((Uint32)v) | ((Uint32)v << 8) | ((Uint32)v << 16) | ((Uint32)a << 24);
        }
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, s);
    SDL_FreeSurface(s);
    if (tex) SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

/* ---- bake all sprites into a single horizontal atlas ----------- */
bool atlas_build(SpriteAtlas* atlas, SDL_Renderer* rend) {
    memset(atlas, 0, sizeof(*atlas));
    int total_w = 0;
    int max_h   = 0;
    for (int i = 0; i < SPRITE_COUNT; i++) {
        total_w += sprite_defs[i].w;
        if (sprite_defs[i].h > max_h) max_h = sprite_defs[i].h;
    }
    atlas->w = total_w;
    atlas->h = max_h;

    Uint32 rmask = 0x000000FF, gmask = 0x0000FF00,
           bmask = 0x00FF0000, amask = 0xFF000000;
    SDL_Surface* surf = SDL_CreateRGBSurface(0, total_w, max_h, 32,
                                             rmask, gmask, bmask, amask);
    if (!surf) return false;
    SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);
    SDL_FillRect(surf, NULL, 0);

    Uint32* px = (Uint32*)surf->pixels;
    int xoff = 0;
    for (int i = 0; i < SPRITE_COUNT; i++) {
        const SpriteDef* d = &sprite_defs[i];
        atlas->rects[i].x = xoff;
        atlas->rects[i].y = 0;
        atlas->rects[i].w = d->w;
        atlas->rects[i].h = d->h;
        for (int y = 0; y < d->h; y++) {
            const char* row = d->rows[y];
            for (int x = 0; x < d->w; x++) {
                uint8_t idx = palette_index_from_char(row[x]);
                if (idx == 0) continue;
                px[y * total_w + (xoff + x)] = PALETTE[idx];
            }
        }
        xoff += d->w;
    }
    atlas->texture = SDL_CreateTextureFromSurface(rend, surf);
    SDL_FreeSurface(surf);
    if (!atlas->texture) return false;
    SDL_SetTextureBlendMode(atlas->texture, SDL_BLENDMODE_BLEND);
    return true;
}

void atlas_free(SpriteAtlas* atlas) {
    if (atlas->texture) SDL_DestroyTexture(atlas->texture);
    atlas->texture = NULL;
}

void renderer_clear(Renderer* r, uint32_t color_abgr) {
    Uint8 a = (color_abgr >> 24) & 0xFF;
    Uint8 b = (color_abgr >> 16) & 0xFF;
    Uint8 g = (color_abgr >>  8) & 0xFF;
    Uint8 rr = (color_abgr >>  0) & 0xFF;
    SDL_SetRenderDrawColor(r->rend, rr, g, b, a);
    SDL_RenderClear(r->rend);
}

void renderer_present(Renderer* r) {
    SDL_RenderPresent(r->rend);
}

void draw_sprite(Renderer* r, SpriteID id, float x, float y, int flip) {
    if (id < 0 || id >= SPRITE_COUNT) return;
    const SDL_Rect* src = &r->atlas.rects[id];
    SDL_Rect dst = { (int)x, (int)y, src->w, src->h };
    SDL_RendererFlip f = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(r->rend, r->atlas.texture, src, &dst, 0, NULL, f);
}

void draw_sprite_screen(Renderer* r, SpriteID id, int sx, int sy, int flip) {
    if (id < 0 || id >= SPRITE_COUNT) return;
    const SDL_Rect* src = &r->atlas.rects[id];
    SDL_Rect dst = { sx, sy, src->w, src->h };
    SDL_RendererFlip f = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(r->rend, r->atlas.texture, src, &dst, 0, NULL, f);
}

void draw_rect_screen(Renderer* r, IRect rc, uint32_t color_abgr, int filled) {
    Uint8 a = (color_abgr >> 24) & 0xFF;
    Uint8 b = (color_abgr >> 16) & 0xFF;
    Uint8 g = (color_abgr >>  8) & 0xFF;
    Uint8 rr = (color_abgr >>  0) & 0xFF;
    SDL_SetRenderDrawColor(r->rend, rr, g, b, a);
    SDL_Rect sdlrc = { rc.x, rc.y, rc.w, rc.h };
    if (filled) SDL_RenderFillRect(r->rend, &sdlrc);
    else        SDL_RenderDrawRect(r->rend, &sdlrc);
}

/* ---- void glow: additive radial purple halo ----------------------- *
 * Drawn UNDER the player so the world is lit by their void nature.
 * alpha modulates intensity (combat/parry boosts it).
 */
void draw_void_glow(Renderer* r, int sx, int sy, int radius_px, uint8_t alpha) {
    if (!r->void_glow) return;
    SDL_SetTextureAlphaMod(r->void_glow, alpha);
    SDL_Rect dst = { sx - radius_px, sy - radius_px, radius_px * 2, radius_px * 2 };
    SDL_RenderCopy(r->rend, r->void_glow, NULL, &dst);
}

/* ---- vignette: dark edges, blend mode ---------------------------- */
void draw_vignette(Renderer* r) {
    if (!r->vignette) return;
    SDL_RenderCopy(r->rend, r->vignette, NULL, NULL);
}

/* ---- film grain: tiled noise, offset by tick for animation ------- */
void draw_grain(Renderer* r, int tick) {
    if (!r->grain) return;
    /* tile across the logical viewport; offset by tick for shimmer */
    int off_x = (tick * 3) & 63;
    int off_y = (tick * 5) & 63;
    for (int y = -off_y; y < r->logical_h; y += 64) {
        for (int x = -off_x; x < r->logical_w; x += 64) {
            SDL_Rect dst = { x, y, 64, 64 };
            SDL_RenderCopy(r->rend, r->grain, NULL, &dst);
        }
    }
}
