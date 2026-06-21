#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../art/palette.h"
#include "../art/player_sprite.h"
#include "../art/voidscrew_sprite.h"
#include "../art/enemy_void_crawler.h"
#include "../art/town_tiles.h"

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
    /* Software renderer first; falls back gracefully if no GPU. */
    r->rend = SDL_CreateRenderer(r->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!r->rend) {
        r->rend = SDL_CreateRenderer(r->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!r->rend) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return false;
    }
    SDL_SetRenderDrawBlendMode(r->rend, SDL_BLENDMODE_BLEND);

    if (!atlas_build(&r->atlas, r->rend)) {
        fprintf(stderr, "atlas_build failed\n");
        return false;
    }
    return true;
}

void renderer_free(Renderer* r) {
    atlas_free(&r->atlas);
    if (r->rend)   SDL_DestroyRenderer(r->rend);
    if (r->window) SDL_DestroyWindow(r->window);
    SDL_Quit();
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
