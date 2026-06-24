#include "audio.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

#define PI 3.14159265358979323846f

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Each SFX has a length (in samples) and a synth function. */
static int sfx_length(SfxId id, int sr) {
    float secs;
    switch (id) {
        case SFX_SLASH:           secs = 0.18f; break;
        case SFX_PARRY:           secs = 0.55f; break;
        case SFX_PERFECT_PARRY:   secs = 0.85f; break;
        case SFX_POGO:            secs = 0.22f; break;
        case SFX_HURT:            secs = 0.30f; break;
        case SFX_SPRINT:          secs = 0.40f; break;
        case SFX_KILL:            secs = 0.50f; break;
        case SFX_MENU_MOVE:       secs = 0.08f; break;
        case SFX_MENU_SELECT:     secs = 0.18f; break;
        default: return 0;
    }
    return (int)(secs * sr);
}

/* quick LCG noise so it's deterministic per call */
static float lcg_state = 12345.0f;
static float rand_noise(void) {
    lcg_state = fmodf(lcg_state * 1103515245.0f + 12345.0f, 2147483648.0f);
    return lcg_state / 1073741824.0f - 1.0f;   /* [-1,1] */
}

/* ADSR-ish envelope: attack + decay + release based on t in [0,1] */
static float env_curve(SfxId id, float t) {
    /* t = cursor/length in [0,1] */
    switch (id) {
        case SFX_SLASH:
            /* sharp attack, fast decay */
            if (t < 0.02f) return t / 0.02f;
            return 1.0f - (t - 0.02f) / 0.98f;
        case SFX_PARRY:
            /* slow attack (the "moment"), long release */
            if (t < 0.15f) return t / 0.15f;
            return 1.0f - (t - 0.15f) / 0.85f;
        case SFX_PERFECT_PARRY:
            if (t < 0.10f) return t / 0.10f;
            return powf(1.0f - (t - 0.10f) / 0.90f, 1.5f);
        case SFX_POGO:
            if (t < 0.05f) return t / 0.05f;
            return powf(1.0f - (t - 0.05f) / 0.95f, 2.0f);
        case SFX_HURT:
            return powf(1.0f - t, 1.3f);
        case SFX_SPRINT:
            if (t < 0.20f) return t / 0.20f;
            if (t > 0.70f) return (1.0f - t) / 0.30f;
            return 1.0f;
        case SFX_KILL:
            if (t < 0.05f) return t / 0.05f;
            return powf(1.0f - (t - 0.05f) / 0.95f, 1.5f);
        case SFX_MENU_MOVE:
            return powf(1.0f - t, 2.0f);
        case SFX_MENU_SELECT:
            if (t < 0.05f) return t / 0.05f;
            return powf(1.0f - (t - 0.05f) / 0.95f, 1.2f);
        default:
            return 0;
    }
}

/* synth one sample of given SFX at time t (0..1) */
static float synth_sample(SfxId id, float t, int sr) {
    switch (id) {
        case SFX_SLASH: {
            /* noise + descending sine sweep (high to mid) */
            float sweep = 1200.0f * (1.0f - t) + 400.0f;
            float sine = sinf(2.0f * PI * sweep * t * 0.05f);
            float noise = rand_noise() * 0.6f;
            return 0.5f * sine + 0.5f * noise;
        }
        case SFX_PARRY: {
            /* deep void chord: 110Hz + 165Hz (perfect fifth) + slight detune */
            float a = sinf(2.0f * PI * 110.0f * t * 0.55f);
            float b = sinf(2.0f * PI * 165.0f * t * 0.55f) * 0.6f;
            float c = sinf(2.0f * PI * 55.0f  * t * 0.55f) * 0.8f;   /* sub */
            return (a + b + c) / 2.4f;
        }
        case SFX_PERFECT_PARRY: {
            /* brighter chord + shimmer */
            float a = sinf(2.0f * PI * 220.0f * t * 0.85f);
            float b = sinf(2.0f * PI * 330.0f * t * 0.85f) * 0.7f;
            float c = sinf(2.0f * PI * 440.0f * t * 0.85f) * 0.5f;
            float shimmer = sinf(2.0f * PI * 1320.0f * t * 0.85f) * 0.15f * (1.0f - t);
            return (a + b + c + shimmer) / 2.2f;
        }
        case SFX_POGO: {
            /* low thud + dust noise */
            float thud = sinf(2.0f * PI * 80.0f * t * 0.22f) * (1.0f - t);
            float dust = rand_noise() * 0.4f * (1.0f - t);
            return thud * 0.8f + dust;
        }
        case SFX_HURT: {
            /* detuned sine descending */
            float freq = 400.0f * (1.0f - t * 0.6f);
            float a = sinf(2.0f * PI * freq * t * 0.30f);
            float b = sinf(2.0f * PI * (freq * 1.01f) * t * 0.30f) * 0.7f;
            return (a + b) / 1.7f;
        }
        case SFX_SPRINT: {
            /* void hum: low sine + filtered noise */
            float hum = sinf(2.0f * PI * 60.0f * t * 0.40f) * 0.7f;
            float hum2 = sinf(2.0f * PI * 90.0f * t * 0.40f) * 0.5f;
            float noise = rand_noise() * 0.3f;
            return hum + hum2 + noise;
        }
        case SFX_KILL: {
            /* noise sweep + low boom */
            float boom = sinf(2.0f * PI * 50.0f * t * 0.50f) * (1.0f - t);
            float sweep = 800.0f * (1.0f - t);
            float sine = sinf(2.0f * PI * sweep * t * 0.10f) * (1.0f - t);
            float noise = rand_noise() * 0.5f * (1.0f - t);
            return (boom + sine + noise) / 2.0f;
        }
        case SFX_MENU_MOVE: {
            float freq = 600.0f;
            return sinf(2.0f * PI * freq * t * 0.08f);
        }
        case SFX_MENU_SELECT: {
            float freq = 800.0f + 400.0f * t;
            return sinf(2.0f * PI * freq * t * 0.18f);
        }
        default:
            return 0;
    }
}

/* find a voice slot to steal: prefer inactive, else oldest cursor */
static int pick_voice_slot(Audio* a) {
    int best = 0;
    int best_cursor = -1;
    for (int i = 0; i < MAX_VOICES; i++) {
        if (!a->voices[i].active) return i;
        if (a->voices[i].cursor > best_cursor) {
            best_cursor = a->voices[i].cursor;
            best = i;
        }
    }
    return best;
}

void audio_play(Audio* a, SfxId id) {
    if (!a->dev || a->muted) return;
    if (id <= SFX_NONE || id >= SFX_COUNT) return;
    int slot = pick_voice_slot(a);
    Voice* v = &a->voices[slot];
    v->active = true;
    v->id = id;
    v->sample_rate = a->spec.freq;
    v->cursor = 0;
    v->length = sfx_length(id, v->sample_rate);
    v->gain = 0.7f;
}

void audio_set_muted(Audio* a, bool muted) { a->muted = muted; }

/* The SDL audio callback. Fills `stream` with `len` bytes of mixed audio. */
static void audio_callback(void* userdata, Uint8* stream, int len) {
    Audio* a = (Audio*)userdata;
    int sr = a->spec.freq;
    int channels = a->spec.channels;
    int bytes_per_sample = 2;   /* S16 */
    int frames = len / (channels * bytes_per_sample);

    /* clear */
    memset(stream, 0, len);

    Sint16* out = (Sint16*)stream;
    for (int frame = 0; frame < frames; frame++) {
        float mix = 0.0f;
        for (int i = 0; i < MAX_VOICES; i++) {
            Voice* v = &a->voices[i];
            if (!v->active) continue;
            float t = (float)v->cursor / (float)v->length;
            if (t >= 1.0f) {
                v->active = false;
                continue;
            }
            float s = synth_sample(v->id, t, sr);
            float env = env_curve(v->id, t);
            mix += s * env * v->gain;
            v->cursor++;
        }
        /* clamp */
        if (mix > 1.0f)  mix = 1.0f;
        if (mix < -1.0f) mix = -1.0f;
        Sint16 sample = (Sint16)(mix * 12000);
        for (int ch = 0; ch < channels; ch++) {
            out[frame * channels + ch] = sample;
        }
    }
}

bool audio_init(Audio* a) {
    memset(a, 0, sizeof(*a));
    SDL_AudioSpec want;
    memset(&want, 0, sizeof(want));
    want.freq = 22050;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 512;
    want.callback = audio_callback;
    want.userdata = a;

    a->dev = SDL_OpenAudioDevice(NULL, 0, &want, &a->spec, 0);
    if (!a->dev) {
        /* audio is optional; the game should still run without it */
        return false;
    }
    SDL_PauseAudioDevice(a->dev, 0);   /* start playing */
    return true;
}

void audio_free(Audio* a) {
    if (a->dev) {
        SDL_PauseAudioDevice(a->dev, 1);
        SDL_CloseAudioDevice(a->dev);
        a->dev = 0;
    }
}
