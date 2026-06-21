#include "hotreload.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
#include <sys/inotify.h>
#include <poll.h>
#define HAVE_INOTIFY 1
#else
#define HAVE_INOTIFY 0
#endif

bool hotreload_init(HotReload* hr, const char* tunables_path,
                    const char* levels_dir, HotReloadCb cb, void* user) {
    memset(hr, 0, sizeof(*hr));
    hr->cb = cb;
    hr->user = user;
    strncpy(hr->levels_dir, levels_dir, sizeof(hr->levels_dir)-1);

#if HAVE_INOTIFY
    hr->inotify_fd = inotify_init1(IN_NONBLOCK);
    if (hr->inotify_fd < 0) return false;

    if (tunables_path) {
        hr->watch_tunables = inotify_add_watch(hr->inotify_fd, tunables_path,
            IN_MODIFY | IN_CLOSE_WRITE);
    }
    if (levels_dir) {
        hr->watch_levels = inotify_add_watch(hr->inotify_fd, levels_dir,
            IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE);
    }
    return true;
#else
    (void)tunables_path; (void)levels_dir;
    return false;
#endif
}

void hotreload_poll(HotReload* hr) {
#if HAVE_INOTIFY
    if (hr->inotify_fd < 0) return;
    /* drain all pending events */
    char buf[4096] __attribute__((aligned(8)));
    while (true) {
        ssize_t n = read(hr->inotify_fd, buf, sizeof(buf));
        if (n <= 0) break;
        /* process events; we just need to know SOMETHING changed */
        for (char* p = buf; p < buf + n; ) {
            struct inotify_event* ev = (struct inotify_event*)p;
            if (hr->cb) {
                if (ev->wd == hr->watch_tunables) {
                    hr->cb("tunables.txt", hr->user);
                } else if (ev->wd == hr->watch_levels) {
                    /* notify with the changed filename */
                    char full[512];
                    if (ev->len > 0) {
                        snprintf(full, sizeof(full), "%s/%s", hr->levels_dir, ev->name);
                    } else {
                        strncpy(full, hr->levels_dir, sizeof(full)-1);
                    }
                    hr->cb(full, hr->user);
                }
            }
            p += sizeof(struct inotify_event) + ev->len;
        }
    }
#endif
}

void hotreload_free(HotReload* hr) {
#if HAVE_INOTIFY
    if (hr->inotify_fd >= 0) {
        if (hr->watch_tunables >= 0) inotify_rm_watch(hr->inotify_fd, hr->watch_tunables);
        if (hr->watch_levels   >= 0) inotify_rm_watch(hr->inotify_fd, hr->watch_levels);
        close(hr->inotify_fd);
        hr->inotify_fd = -1;
    }
#endif
}
