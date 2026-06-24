#include "scene.h"
#include "world.h"
#include "audio.h"
#include "input.h"
#include "render.h"
#include "config.h"
#include "i18n.h"
#include "tunables.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * TITLE scene: simple static title screen. Press Z/X/Enter to advance to
 * the intro cutscene.
 */

typedef struct {
    int  ticks;
    bool pressed_any;
} TitleState;

static void title_init(Scene* self, void* args) {
    (void)args;
    TitleState* st = calloc(1, sizeof(TitleState));
    self->state = st;
}

static SceneUpdate title_update(Scene* self, const Input* in, float dt) {
    (void)dt;
    TitleState* st = self->state;
    st->ticks++;
    if (in->pressed[BTN_JUMP] || in->pressed[BTN_ATTACK]
        || in->pressed[BTN_PAUSE]) {
        SceneUpdate u = { SCENE_REPLACE, &scene_intro, NULL };
        return u;
    }
    if (in->pressed[BTN_QUIT]) {
        SceneUpdate u = { SCENE_POP, NULL, NULL };
        return u;
    }
    SceneUpdate u = { SCENE_CONTINUE, NULL, NULL };
    return u;
}

static void title_draw(Scene* self, Renderer* r) {
    TitleState* st = self->state;
    (void)st;
    /* dark void background */
    renderer_clear(r, 0xFF06060A);
    /* Title text would go here once we have a font. For now, draw two
     * placeholder rectangles to suggest a title layout. */
    IRect title_bar = { LOGICAL_W/2 - 100, LOGICAL_H/2 - 40, 200, 30 };
    draw_rect_screen(r, title_bar, 0xFFA42D7A, 1);
    IRect subtitle_bar = { LOGICAL_W/2 - 80, LOGICAL_H/2 + 10, 160, 8 };
    /* blink the "press to start" */
    if ((st->ticks / 30) & 1) {
        draw_rect_screen(r, subtitle_bar, 0xFF88CCFF, 1);
    }
}

static void title_free(Scene* self) {
    free(self->state);
    self->state = NULL;
}

const SceneVTable scene_title = {
    .init = title_init, .update = title_update,
    .draw = title_draw, .free = title_free,
    .resume = NULL, .pause = NULL,
};

/* ----------------------------------------------------------------- *
 * INTRO scene: plays the opening dialogue from the design doc.
 *
 *   "You… who are you? No…. what are you? Are you one of them?
 *    Maybe you are… but you seem small….hmmmm…"
 *   "Why are you here?"        "..."
 *   "How old are you?"          "..."
 *   "Never mind"
 *
 * Then advances to the world scene.
 * ----------------------------------------------------------------- */

typedef struct {
    int   line_index;
    int   char_index;       /* typewriter cursor */
    int   ticks_per_char;
    int   line_hold;        /* ticks to wait after line completes */
} IntroState;

static const char* intro_lines[] = {
    _("You... who are you?"),
    _("No.... what are you?"),
    _("Are you one of them?"),
    _("Maybe you are... but you seem small....hmmmm..."),
    _("Why are you here?"),
    _("..."),
    _("How old are you?"),
    _("..."),
    _("Never mind"),
};
#define INTRO_LINE_COUNT (sizeof(intro_lines)/sizeof(intro_lines[0]))

static void intro_init(Scene* self, void* args) {
    (void)args;
    IntroState* st = calloc(1, sizeof(IntroState));
    st->line_index = 0;
    st->char_index = 0;
    st->ticks_per_char = 3;
    st->line_hold = 0;
    self->state = st;
}

static SceneUpdate intro_update(Scene* self, const Input* in, float dt) {
    (void)dt;
    IntroState* st = self->state;

    if (in->pressed[BTN_QUIT]) {
        SceneUpdate u = { SCENE_POP, NULL, NULL };
        return u;
    }

    if (st->line_hold > 0) {
        st->line_hold--;
        if (st->line_hold == 0) {
            st->line_index++;
            st->char_index = 0;
            if (st->line_index >= (int)INTRO_LINE_COUNT) {
                /* advance to the world scene */
                SceneUpdate u = { SCENE_REPLACE, &scene_world, (void*)"levels/town.txt" };
                return u;
            }
        }
    } else {
        /* typewriter: advance char_index every ticks_per_char */
        if ((st->line_index + st->char_index) % st->ticks_per_char == 0) {
            /* advancing handled below */
        }
        st->char_index++;
        const char* line = intro_lines[st->line_index];
        if (st->char_index > (int)strlen(line)) {
            st->char_index = (int)strlen(line);
            st->line_hold = 40;   /* ~0.7s hold after line completes */
        }
        /* pressing jump skips to end of current line / next line */
        if (in->pressed[BTN_JUMP] || in->pressed[BTN_ATTACK]) {
            if (st->char_index < (int)strlen(line)) {
                st->char_index = (int)strlen(line);
            } else {
                st->line_hold = 1;
            }
        }
    }

    SceneUpdate u = { SCENE_CONTINUE, NULL, NULL };
    return u;
}

static void intro_draw(Scene* self, Renderer* r) {
    IntroState* st = self->state;
    renderer_clear(r, 0xFF06060A);

    /* draw the current line as a row of small rects (one per char) */
    const char* line = intro_lines[st->line_index];
    int n = st->char_index;
    if (n > (int)strlen(line)) n = (int)strlen(line);
    int char_w = 8;
    int total_w = (int)strlen(line) * char_w;
    int x = LOGICAL_W/2 - total_w/2;
    int y = LOGICAL_H/2;
    for (int i = 0; i < n; i++) {
        unsigned char c = (unsigned char)line[i];
        /* hash char to a color for visual variety (placeholder until font) */
        Color col = 0xFFE0E0E0;
        if (c == '.' || c == ',') col = 0xFF888899;
        IRect rc = { x + i * char_w, y, char_w - 1, 12 };
        draw_rect_screen(r, rc, col, 1);
    }
    /* small child silhouette in the lower-left (placeholder) */
    IRect child = { LOGICAL_W/4, LOGICAL_H - 60, 32, 48 };
    draw_rect_screen(r, child, 0xFF3A3A4E, 1);
}

static void intro_free(Scene* self) {
    free(self->state);
    self->state = NULL;
}

const SceneVTable scene_intro = {
    .init = intro_init, .update = intro_update,
    .draw = intro_draw, .free = intro_free,
    .resume = NULL, .pause = NULL,
};

/* ----------------------------------------------------------------- *
 * WORLD scene: wraps a World struct. Loads a level path passed as args.
 * ----------------------------------------------------------------- */

typedef struct {
    struct World world;
    char   level_path[256];
    Audio* audio;
} WorldSceneState;

static void world_scene_init(Scene* self, void* args) {
    WorldSceneState* st = calloc(1, sizeof(WorldSceneState));
    self->state = st;
    const char* path = args ? (const char*)args : "levels/town.txt";
    strncpy(st->level_path, path, sizeof(st->level_path)-1);
    /* audio is supplied via the scene stack */
    /* we'll set it on first update from the stack's audio ptr; for now
     * initialize world with NULL and patch on first update */
    world_init(&st->world, st->level_path, NULL);
}

static SceneUpdate world_scene_update(Scene* self, const Input* in, float dt) {
    WorldSceneState* st = self->state;

    /* toggle pause */
    if (in->pressed[BTN_PAUSE]) {
        /* push pause scene on top of us */
        SceneUpdate u = { SCENE_CONTINUE, &scene_pause, NULL };
        /* we need to push, not replace. SceneUpdate doesn't have a PUSH
         * result; instead, the scene stack treats SCENE_REPLACE with a
         * different semantic. For simplicity, treat pressing P as
         * "push pause" via a small hack: we set next_scene but mark
         * SCENE_CONTINUE. The stack won't push for us, so we'll do it
         * manually below by stashing a flag. */
        /* Actually, for the prototype, let's just toggle a paused flag
         * inside WorldSceneState instead of using a separate scene. */
        st->world.player.hitstop = -1;   /* sentinel: paused */
        (void)u;
    }
    if (st->world.player.hitstop < 0) {
        /* paused: wait for unpause */
        if (in->pressed[BTN_JUMP] || in->pressed[BTN_PAUSE]) {
            st->world.player.hitstop = 0;
        }
        if (in->pressed[BTN_QUIT]) {
            SceneUpdate u = { SCENE_REPLACE, &scene_title, NULL };
            return u;
        }
        SceneUpdate u = { SCENE_CONTINUE, NULL, NULL };
        return u;
    }

    world_update(&st->world, in, dt);
    if (in->pressed[BTN_QUIT]) {
        SceneUpdate u = { SCENE_REPLACE, &scene_title, NULL };
        return u;
    }
    SceneUpdate u = { SCENE_CONTINUE, NULL, NULL };
    return u;
}

static void world_scene_draw(Scene* self, Renderer* r) {
    WorldSceneState* st = self->state;
    world_draw(&st->world, r);
    if (st->world.player.hitstop < 0) {
        /* paused overlay */
        renderer_clear(r, 0x80000000);
        IRect rc = { LOGICAL_W/2 - 50, LOGICAL_H/2 - 10, 100, 20 };
        draw_rect_screen(r, rc, 0xFFA42D7A, 1);
    }
}

static void world_scene_free(Scene* self) {
    WorldSceneState* st = self->state;
    (void)st;
    free(self->state);
    self->state = NULL;
}

/* Called by scene_stack when audio becomes available. */
void world_scene_set_audio(Scene* self, Audio* a) {
    WorldSceneState* st = self->state;
    st->audio = a;
    st->world.audio = a;
}

const SceneVTable scene_world = {
    .init = world_scene_init, .update = world_scene_update,
    .draw = world_scene_draw, .free = world_scene_free,
    .resume = NULL, .pause = NULL,
};

/* ----------------------------------------------------------------- *
 * PAUSE scene: not yet implemented as a separate scene; world scene
 * handles pause internally for now. This stub exists so future code
 * can push a real pause overlay.
 * ----------------------------------------------------------------- */

static void pause_init(Scene* self, void* args) { (void)self; (void)args; }
static SceneUpdate pause_update(Scene* self, const Input* in, float dt) {
    (void)self; (void)dt;
    if (in->pressed[BTN_PAUSE] || in->pressed[BTN_JUMP]) {
        SceneUpdate u = { SCENE_POP, NULL, NULL };
        return u;
    }
    SceneUpdate u = { SCENE_CONTINUE, NULL, NULL };
    return u;
}
static void pause_draw(Scene* self, Renderer* r) {
    (void)self;
    renderer_clear(r, 0x80000000);
}
static void pause_free(Scene* self) { (void)self; }

const SceneVTable scene_pause = {
    .init = pause_init, .update = pause_update,
    .draw = pause_draw, .free = pause_free,
    .resume = NULL, .pause = NULL,
};
