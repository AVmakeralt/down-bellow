#include "config.h"
#include "input.h"
#include "render.h"
#include "world.h"
#include "audio.h"
#include "scene.h"
#include "tunables.h"
#include "hotreload.h"
#include "i18n.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- globals owned by main ---- */
static Audio       g_audio;
static HotReload   g_hotreload;

static void on_file_changed(const char* path, void* user) {
    (void)user;
    if (strstr(path, "tunables.txt")) {
        if (tunables_reload(&g_tunables, "tunables.txt")) {
            fprintf(stderr, "[hotreload] tunables.txt reloaded\n");
        }
    } else if (strstr(path, "levels/")) {
        fprintf(stderr, "[hotreload] level changed: %s (restart to apply)\n", path);
        /* For the prototype we don't live-reload the level (would need to
         * preserve player state across reload). Just log it. */
    }
}

/* If non-zero, write a BMP of this frame index and quit. */
static int g_screenshot_frame = -1;
static const char* g_screenshot_path = NULL;
static int g_auto_walk = 0;
static int g_force_debug = 0;
static int g_skip_title = 0;
static int g_save_at_tick = -1;   /* inject BTN_SAVE press at this tick */

int main(int argc, char** argv) {
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
        } else if (strcmp(argv[i], "--skip-title") == 0) {
            g_skip_title = 1;
        } else if (strcmp(argv[i], "--save-at") == 0 && i + 1 < argc) {
            g_save_at_tick = atoi(argv[i+1]);
            i += 1;
        }
    }

    /* init tunables BEFORE renderer (some code reads them at init) */
    tunables_load_defaults(&g_tunables);
    tunables_load_file(&g_tunables, "tunables.txt");

    Renderer rend;
    if (!renderer_init(&rend, WINDOW_W, WINDOW_H)) {
        fprintf(stderr, "Failed to initialize renderer.\n");
        return 1;
    }
    if (g_force_debug) rend.debug = true;

    /* audio (optional; game runs without it) */
    bool have_audio = audio_init(&g_audio);
    if (!have_audio) {
        fprintf(stderr, "[audio] init failed; running silent\n");
    }

    /* hot reload */
    if (!hotreload_init(&g_hotreload, "tunables.txt", "levels",
                        on_file_changed, NULL)) {
        fprintf(stderr, "[hotreload] init failed (non-Linux?); disabled\n");
    }

    /* init SDL2 gamepad subsystem (input.c opens controllers on its own) */
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "[gamepad] init failed: %s\n", SDL_GetError());
    }

    /* build scene stack */
    SceneStack stack;
    scene_stack_init(&stack, &g_audio);
    if (g_skip_title) {
        scene_push(&stack, &scene_world, (void*)"levels/town.txt");
    } else {
        scene_push(&stack, &scene_title, NULL);
    }

    Input input;
    input_init(&input);

    /* fixed-timestep accumulator */
    const float fixed_dt = 1.0f / FIXED_HZ;
    Uint64 perf_freq = SDL_GetPerformanceFrequency();
    Uint64 last      = SDL_GetPerformanceCounter();
    float  accum     = 0.0f;
    bool   slowmo    = false;
    int    frame_idx = 0;

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
        input_poll_gamepad(&input);
        if (input.pressed[BTN_QUIT]) running = false;
        if (input.pressed[BTN_DEBUG])  rend.debug  = !rend.debug;
        if (input.pressed[BTN_SLOWMO]) slowmo      = !slowmo;

        /* hot-reload poll (cheap; non-blocking) */
        hotreload_poll(&g_hotreload);

        /* headless demo auto-walk: only applies if top scene is WORLD */
        if (g_auto_walk > 0) {
            input.down[BTN_RIGHT] = true;
            if (g_auto_walk < 240 && (g_auto_walk % 30) == 0) {
                input.pressed[BTN_ATTACK] = true;
            }
            g_auto_walk--;
            if (g_auto_walk == 0) input.down[BTN_RIGHT] = false;
        }

        /* inject a manual save press at the requested tick (for testing) */
        if (g_save_at_tick >= 0 && frame_idx >= g_save_at_tick) {
            input.pressed[BTN_SAVE] = true;
            g_save_at_tick = -1;   /* one-shot */
        }

        /* ---- fixed ticks ---- */
        int ticks_this_frame = 0;
        while (accum >= fixed_dt && ticks_this_frame < 5) {
            scene_update(&stack, &input, fixed_dt);
            accum -= fixed_dt;
            ticks_this_frame++;
        }
        if (stack.top < 0) running = false;   /* all scenes popped */

        /* ---- render ---- */
        scene_draw(&stack, &rend);

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

    /* shutdown */
    scene_stack_shutdown(&stack);
    input_shutdown(&input);
    hotreload_free(&g_hotreload);
    audio_free(&g_audio);
    renderer_free(&rend);
    return 0;
}
