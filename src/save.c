#include "save.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void save_init(SaveData* s) {
    memset(s, 0, sizeof(*s));
    s->magic = SAVE_MAGIC;
    s->version = SAVE_VERSION;
    s->play_time_ticks = 0;
    s->save_count = 0;
    s->last_reason = SAVE_REASON_NONE;
    s->player_hp = 5;
    s->player_facing = 1;
    s->player_kenotita = 100.0f;
    strncpy(s->level_path, "levels/town.txt", sizeof(s->level_path) - 1);
}

bool save_write(const SaveData* s, const char* path) {
    /* Write atomically: write to .tmp then rename, so a crash mid-write
     * doesn't corrupt the existing save. */
    char tmp_path[512];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path);

    FILE* f = fopen(tmp_path, "wb");
    if (!f) return false;
    size_t n = fwrite(s, sizeof(SaveData), 1, f);
    fflush(f);
    fclose(f);
    if (n != 1) {
        remove(tmp_path);
        return false;
    }
    if (rename(tmp_path, path) != 0) {
        /* rename can fail across filesystems; fall back to copy */
        FILE* src = fopen(tmp_path, "rb");
        FILE* dst = fopen(path, "wb");
        if (!src || !dst) {
            if (src) fclose(src);
            if (dst) fclose(dst);
            remove(tmp_path);
            return false;
        }
        char buf[4096];
        size_t r;
        while ((r = fread(buf, 1, sizeof(buf), src)) > 0) {
            fwrite(buf, 1, r, dst);
        }
        fclose(src);
        fclose(dst);
        remove(tmp_path);
    }
    return true;
}

bool save_read(SaveData* s, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    size_t n = fread(s, sizeof(SaveData), 1, f);
    fclose(f);
    if (n != 1) return false;
    /* validate */
    if (s->magic != SAVE_MAGIC) {
        memset(s, 0, sizeof(*s));
        return false;
    }
    if (s->version != SAVE_VERSION) {
        /* For prototype: reject incompatible versions. Future: migrate. */
        memset(s, 0, sizeof(*s));
        return false;
    }
    return true;
}

bool save_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

void save_delete(const char* path) {
    remove(path);
}

void save_capture(SaveData* s,
                  const char* level_path,
                  float px, float py, int facing,
                  int hp, float kenotita,
                  int siblings, int anchors,
                  uint32_t play_ticks, uint32_t save_count,
                  SaveReason reason) {
    s->magic = SAVE_MAGIC;
    s->version = SAVE_VERSION;
    s->play_time_ticks = play_ticks;
    s->save_count = save_count + 1;   /* this save is the (count+1)-th */
    s->last_reason = reason;
    s->player_x = px;
    s->player_y = py;
    s->player_facing = facing;
    s->player_hp = hp;
    s->player_kenotita = kenotita;
    s->player_siblings_absorbed = siblings;
    s->player_anchors_shattered = anchors;
    strncpy(s->level_path, level_path, sizeof(s->level_path) - 1);
    s->level_path[sizeof(s->level_path) - 1] = '\0';
}

const char* save_reason_label(SaveReason r) {
    switch (r) {
        case SAVE_REASON_AUTO_PLACE: return "Saved (checkpoint)";
        case SAVE_REASON_AUTO_TIME:  return "Saved (auto)";
        case SAVE_REASON_MANUAL:     return "Saved";
        case SAVE_REASON_NONE:
        default:                      return "";
    }
}
