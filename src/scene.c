#include "scene.h"

#include <string.h>
#include <assert.h>

void scene_stack_init(SceneStack* s, Audio* a) {
    memset(s, 0, sizeof(*s));
    s->top = -1;
    s->audio = a;
}

void scene_stack_shutdown(SceneStack* s) {
    while (s->top >= 0) {
        scene_pop(s);
    }
}

bool scene_push(SceneStack* s, const SceneVTable* vt, void* args) {
    if (s->top + 1 >= SCENE_STACK_MAX) return false;
    /* pause the current top */
    if (s->top >= 0 && s->scenes[s->top].vtable && s->scenes[s->top].vtable->pause) {
        s->scenes[s->top].vtable->pause(&s->scenes[s->top]);
    }
    s->top++;
    Scene* sc = &s->scenes[s->top];
    sc->vtable = vt;
    sc->state = NULL;
    if (vt && vt->init) vt->init(sc, args);
    return true;
}

void scene_pop(SceneStack* s) {
    if (s->top < 0) return;
    Scene* sc = &s->scenes[s->top];
    if (sc->vtable && sc->vtable->free) sc->vtable->free(sc);
    s->top--;
    if (s->top >= 0) {
        Scene* below = &s->scenes[s->top];
        if (below->vtable && below->vtable->resume) below->vtable->resume(below);
    }
}

void scene_replace(SceneStack* s, const SceneVTable* vt, void* args) {
    if (s->top < 0) {
        scene_push(s, vt, args);
        return;
    }
    Scene* sc = &s->scenes[s->top];
    if (sc->vtable && sc->vtable->free) sc->vtable->free(sc);
    sc->vtable = vt;
    sc->state = NULL;
    if (vt && vt->init) vt->init(sc, args);
}

void scene_update(SceneStack* s, const Input* in, float dt) {
    if (s->top < 0) return;
    Scene* sc = &s->scenes[s->top];
    if (!sc->vtable || !sc->vtable->update) return;
    SceneUpdate u = sc->vtable->update(sc, in, dt);
    switch (u.result) {
        case SCENE_CONTINUE: break;
        case SCENE_POP:      scene_pop(s); break;
        case SCENE_REPLACE:  scene_replace(s, u.next_scene, u.next_args); break;
    }
}

void scene_draw(SceneStack* s, Renderer* r) {
    if (s->top < 0) return;
    /* draw from bottom up so overlays appear on top */
    for (int i = 0; i <= s->top; i++) {
        Scene* sc = &s->scenes[i];
        if (sc->vtable && sc->vtable->draw) sc->vtable->draw(sc, r);
    }
}
