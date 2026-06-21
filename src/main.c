#include "config.h"
#include "input.h"
#include "render.h"
#include "world.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* If non-zero, write a BMP of this frame index and quit. */
static int g_screenshot_frame = -1;
static const char* g_screenshot_path = NULL;

/* If non-zero, simulate holding RIGHT every tick (for headless demo). */
static int g_auto_walk = 0;

/* If set, force-enable debug overlay for screenshots. */
static int g_force_debug = 0;

int main(int argc, char** argv) {
    /* Parse args:
     *   --shot N path  -> capture frame N to path, then quit
     *   --auto N       -> simulate holding RIGHT for the first N ticks
     *   --debug        -> force-enable debug overlay
     */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--shot") == 0 && i + 2 < argc) {
            g_screenshot_frame = atoi(argv[i+1]);
            g_screenshot_path  = argv[i+2];
            i += 2;
        } else if (strcmp(argv[i], "--auto") == 0 && i + 1 < argc) {
            g_auto_walk = atoi(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--debug") == 0) {
            g_force_debug = 1;
        }
    }

    Renderer rend;
    if (!renderer_init(&rend, WINDOW_W, WINDOW_H)) {
        fprintf(stderr, "Failed to initialize renderer.\n");
        return 1;
    }
    if (g_force_debug) rend.debug = true;

    Input input;
    input_init(&input);

    World world;
    world_init(&world, "levels/town.txt");

    const float fixed_dt = 1.0f / FIXED_HZ;
    Uint64    perf_freq = SDL_GetPerformanceFrequency();
    Uint64    last      = SDL_GetPerformanceCounter();
    float     accum     = 0.0f;
    bool      slowmo    = false;
    int       frame_idx = 0;

    bool running = true;
    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float frame_secs = (float)(now - last) / (float)perf_freq;
        last = now;
        if (frame_secs > 0.25f) frame_secs = 0.25f;
        accum += frame_secs * (slowmo ? 0.25f : 1.0f);

        input_begin_frame(&input);
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            input_handle_event(&input, &e);
        }
        if (input.pressed[BTN_QUIT]) running = false;
        if (input.pressed[BTN_DEBUG])  rend.debug  = !rend.debug;
        if (input.pressed[BTN_SLOWMO]) slowmo      = !slowmo;

        /* Headless demo: force-hold RIGHT for the first N ticks so we can
         * capture screenshots of the player walking toward the enemy.
         * Also auto-attacks periodically once we're close. */
        if (g_auto_walk > 0) {
            input.down[BTN_RIGHT] = true;
            /* attack every ~30 ticks after the first 60 ticks */
            if (g_auto_walk < 240 && (g_auto_walk % 30) == 0) {
                input.pressed[BTN_ATTACK] = true;
            }
            g_auto_walk--;
            if (g_auto_walk == 0) {
                input.down[BTN_RIGHT] = false;
            }
        }

        int ticks_this_frame = 0;
        while (accum >= fixed_dt && ticks_this_frame < 5) {
            world_update(&world, &input);
            accum -= fixed_dt;
            ticks_this_frame++;
        }

        world_draw(&world, &rend);

        /* screenshot capture mode (headless smoke test) */
        if (g_screenshot_frame >= 0 && frame_idx >= g_screenshot_frame) {
            int w = WINDOW_W, h = WINDOW_H;
            SDL_Surface* surf = SDL_CreateRGBSurface(0, w, h, 32,
                0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            if (surf) {
                SDL_RenderReadPixels(rend.rend, NULL,
                    SDL_PIXELFORMAT_ABGR8888,
                    surf->pixels, surf->pitch);
                SDL_SaveBMP(surf, g_screenshot_path);
                SDL_FreeSurface(surf);
                fprintf(stderr, "screenshot saved: %s\n", g_screenshot_path);
            }
            running = false;
        }

        renderer_present(&rend);
        frame_idx++;
    }

    renderer_free(&rend);
    return 0;
}
