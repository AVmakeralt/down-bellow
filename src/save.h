#ifndef TV_SAVE_H
#define TV_SAVE_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Save system.
 *
 * Two files:
 *   save.dat  - player state + world flags (small, ~300 bytes)
 *   (future)  - per-area corruption state, killed-enemy roster
 *
 * Three save triggers:
 *   1. AUTO_PLACE  - player steps onto an 'S' tile in the level. One-shot
 *                    per entry (must leave + re-enter to re-trigger).
 *   2. AUTO_TIME   - every 10 minutes of playtime (AUTOSAVE_INTERVAL_TICKS).
 *   3. MANUAL      - player presses F5 (BTN_SAVE). Always works, but
 *                    cooldown-gated to prevent spam.
 *
 * Save format: fixed struct, written binary. Magic + version at the top
 * for forward-compat. Endian is platform-native (fine for prototype;
 * would need byte-swap for cross-platform release).
 *
 * Death semantics: dying does NOT auto-save. Player reloads from the
 * last save point. (For prototype, dying just respawns at last save;
 * full death-handling comes later.)
 */

#define SAVE_MAGIC             0x44494F56u   /* 'VOI D' little-endian */
#define SAVE_VERSION           1u
#define SAVE_FILE              "save.dat"
#define AUTOSAVE_INTERVAL_TICKS (10 * 60 * 60)  /* 10 min @ 60 Hz */
#define AUTOSAVE_COOLDOWN_TICKS 180             /* 3 sec between saves  */
#define SAVE_FLASH_TICKS        180             /* 3 sec visual pulse   */

typedef enum {
    SAVE_REASON_NONE = 0,
    SAVE_REASON_AUTO_PLACE,    /* stepped on a save point */
    SAVE_REASON_AUTO_TIME,     /* 10-minute timer fired  */
    SAVE_REASON_MANUAL,        /* F5                     */
} SaveReason;

typedef struct {
    uint32_t    magic;
    uint32_t    version;
    uint32_t    play_time_ticks;     /* total ticks played across sessions */
    uint32_t    save_count;          /* how many saves total                */
    SaveReason  last_reason;         /* why the last save happened          */

    /* Player */
    float       player_x;
    float       player_y;
    int         player_facing;
    int         player_hp;
    float       player_kenotita;
    int         player_siblings_absorbed;  /* bitmask: 1=Moth, 2=Lost, 4=Keres */
    int         player_anchors_shattered;  /* bitmask of shattered anchors     */

    /* World */
    char        level_path[256];
} SaveData;

void  save_init(SaveData* s);
bool  save_write(const SaveData* s, const char* path);
bool  save_read(SaveData* s, const char* path);
bool  save_exists(const char* path);
void  save_delete(const char* path);

/* Capture current state into a SaveData. */
void  save_capture(SaveData* s,
                   const char* level_path,
                   float px, float py, int facing,
                   int hp, float kenotita,
                   int siblings, int anchors,
                   uint32_t play_ticks, uint32_t save_count,
                   SaveReason reason);

/* Human-readable label for the reason (for the save-flash overlay). */
const char* save_reason_label(SaveReason r);

#endif /* TV_SAVE_H */
