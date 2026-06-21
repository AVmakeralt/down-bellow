#ifndef TV_HOTRELOAD_H
#define TV_HOTRELOAD_H

#include <stdbool.h>

/*
 * Hot reload. Watches levels (txt files) and tunables.txt via inotify (Linux).
 * On any file change, fires the registered reload callback.
 *
 * Why not just recompile? Recompiling loses game state. With inotify we
 * can keep the game running, edit the level file, save, and see the
 * change live. This is the single biggest dev-velocity win.
 *
 * Usage:
 *     HotReload hr;
 *     if (hotreload_init(&hr, "tunables.txt", my_reload_cb)) { ... }
 *     while (running) {
 *         hotreload_poll(&hr);   // fires cb if file changed
 *         ...
 *     }
 *     hotreload_free(&hr);
 *
 * Non-Linux platforms: hotreload_init returns false; poll is a no-op.
 */

typedef void (*HotReloadCb)(const char* path, void* user);

typedef struct {
    int         inotify_fd;
    int         watch_tunables;
    int         watch_levels;
    HotReloadCb cb;
    void*       user;
    char        levels_dir[256];
} HotReload;

bool hotreload_init(HotReload* hr, const char* tunables_path,
                    const char* levels_dir, HotReloadCb cb, void* user);
void hotreload_poll(HotReload* hr);
void hotreload_free(HotReload* hr);

#endif /* TV_HOTRELOAD_H */
