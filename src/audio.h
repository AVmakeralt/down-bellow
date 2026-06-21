#ifndef TV_AUDIO_H
#define TV_AUDIO_H

#include <stdbool.h>
#include <SDL2/SDL.h>

/*
 * Procedural audio. No external samples; all SFX are synthesized at runtime
 * from sine + noise primitives via a single SDL_AudioSpec callback.
 *
 * The callback mixes up to N simultaneous "voices", each with its own
 * envelope and waveform. Voices are fire-and-forget; the callback decays
 * them and recycles the slot.
 *
 * Sound design notes:
 *   slash      short noise burst + high sine sweep down
 *   parry      deep sine chord, slow attack, long release (the "moment")
 *   perfect    brighter chord, longer release, +high shimmer
 *   pogo       low thud (sine 80hz) + dust noise
 *   hurt       detuned sine descending
 *   sprint     void hum: low sine + filtered noise loop
 *   kill       noise sweep + low boom
 *   menu_move  short blip
 */

typedef enum {
    SFX_NONE = 0,
    SFX_SLASH,
    SFX_PARRY,
    SFX_PERFECT_PARRY,
    SFX_POGO,
    SFX_HURT,
    SFX_SPRINT,
    SFX_KILL,
    SFX_MENU_MOVE,
    SFX_MENU_SELECT,
    SFX_COUNT
} SfxId;

typedef struct {
    bool  active;
    SfxId id;
    int   sample_rate;
    int   cursor;        /* sample index */
    int   length;        /* total samples */
    float gain;
} Voice;

#define MAX_VOICES 8

typedef struct {
    SDL_AudioDeviceID dev;
    SDL_AudioSpec     spec;
    Voice             voices[MAX_VOICES];
    bool              muted;
} Audio;

bool audio_init(Audio* a);
void audio_free(Audio* a);

/* Fire-and-forget. Picks a free voice slot (steals the oldest if full). */
void audio_play(Audio* a, SfxId id);

void audio_set_muted(Audio* a, bool muted);

#endif /* TV_AUDIO_H */
