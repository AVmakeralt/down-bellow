#ifndef TV_SCENE_H
#define TV_SCENE_H

#include "input.h"
#include "render.h"
#include "audio.h"

/*
 * Scene stack. Each scene is a self-contained game phase with its own
 * init/update/draw/free. The stack lets us push PAUSE over WORLD, or
 * DIALOGUE over a BOSS_FIGHT, without losing state.
 *
 * Lifecycle:
 *   scene_push(s, &scene_world, level_path, audio)
 *       -> calls scene_world.init(s, args)
 *   scene_pop(s)  -> calls top scene's free, removes from stack
 *   scene_replace(s, &scene_xxx, args)  -> pop + push atomically
 *
 * Scene.update returns a SceneResult that the stack interprets:
 *   SCENE_CONTINUE   keep this scene on top
 *   SCENE_POP        pop self
 *   SCENE_REPLACE    replace self with the scene pointed to by next_scene
 */

typedef enum {
    SCENE_CONTINUE = 0,
    SCENE_POP,
    SCENE_REPLACE,
} SceneResult;

typedef struct Scene Scene;
typedef struct SceneStack SceneStack;
typedef struct SceneVTable SceneVTable;

typedef struct {
    SceneResult result;
    const SceneVTable* next_scene;   /* for SCENE_REPLACE */
    void*       next_args;     /* passed to next_scene.init */
} SceneUpdate;

struct SceneVTable {
    /* Called when this scene is pushed. args is caller-supplied. */
    void (*init)(Scene* self, void* args);
    /* Called every fixed tick. */
    SceneUpdate (*update)(Scene* self, const Input* in, float dt);
    /* Called every render frame. */
    void (*draw)(Scene* self, Renderer* r);
    /* Called when this scene is popped or replaced. */
    void (*free)(Scene* self);
    /* Called when the scene below this one (on the stack) is popped
     * (e.g. resume after pause). Optional. */
    void (*resume)(Scene* self);
    /* Called when another scene is pushed on top of this one. Optional. */
    void (*pause)(Scene* self);
};

struct Scene {
    const SceneVTable* vtable;
    void* state;            /* scene-specific state, allocated by init */
};

#define SCENE_STACK_MAX 8

struct SceneStack {
    Scene  scenes[SCENE_STACK_MAX];
    int    top;             /* index of top scene, -1 if empty */
    Audio* audio;
};

void scene_stack_init(SceneStack* s, Audio* a);
void scene_stack_shutdown(SceneStack* s);

bool scene_push(SceneStack* s, const SceneVTable* vt, void* args);
void scene_pop(SceneStack* s);
void scene_replace(SceneStack* s, const SceneVTable* vt, void* args);

void scene_update(SceneStack* s, const Input* in, float dt);
void scene_draw(SceneStack* s, Renderer* r);

/* ---- built-in scenes (defined in scene_*.c) ---- */
extern const SceneVTable scene_title;
extern const SceneVTable scene_intro;
extern const SceneVTable scene_world;
extern const SceneVTable scene_pause;

#endif /* TV_SCENE_H */
