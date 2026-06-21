#include "tunables.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

Tunables g_tunables;

void tunables_load_defaults(Tunables* t) {
    t->gravity             = GRAVITY;
    t->move_speed          = MOVE_SPEED;
    t->jump_velocity       = JUMP_VELOCITY;
    t->coyote_time         = COYOTE_TIME;
    t->jump_buffer         = JUMP_BUFFER;
    t->pogo_velocity       = POGO_VELOCITY;

    t->parry_freeze_ticks  = PARRY_FREEZE_TICKS;
    t->parry_perfect_window = PARRY_PERFECT_WINDOW;
    t->parry_stun_ticks    = PARRY_STUN_TICKS;

    t->hitstop_hit         = HITSTOP_TICKS_HIT;
    t->hitstop_parry       = HITSTOP_TICKS_PARRY;
    t->hitstop_perfect     = HITSTOP_TICKS_PERFECT;
    t->hitstop_kill        = HITSTOP_TICKS_KILL;

    t->shake_max_pixels    = SHAKE_MAX_PIXELS;
    t->shake_decay         = SHAKE_DECAY;
    t->shake_trauma_hit    = SHAKE_TRAUMA_HIT;
    t->shake_trauma_parry  = SHAKE_TRAUMA_PARRY;
    t->shake_trauma_perfect= SHAKE_TRAUMA_PERFECT;

    t->sprint_speed        = SPRINT_SPEED;
    t->sprint_duration_ticks = SPRINT_DURATION_TICKS;
    t->kenotita_regen      = KENOTITA_REGEN;
    t->kenotita_sprint_cost= KENOTITA_SPRINT_COST;
}

bool tunables_load_file(Tunables* t, const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return false;

    char line[256];
    bool ok = true;
    while (fgets(line, sizeof(line), f)) {
        /* strip comments and newline */
        char* hash = strchr(line, '#');
        if (hash) *hash = '\0';
        char key[64]; char val[64];
        if (sscanf(line, "%63s %63s", key, val) != 2) continue;

        /* float keys */
        if      (!strcmp(key, "gravity"))              t->gravity = (float)atof(val);
        else if (!strcmp(key, "move_speed"))           t->move_speed = (float)atof(val);
        else if (!strcmp(key, "jump_velocity"))        t->jump_velocity = (float)atof(val);
        else if (!strcmp(key, "pogo_velocity"))        t->pogo_velocity = (float)atof(val);
        else if (!strcmp(key, "shake_max_pixels"))     t->shake_max_pixels = (float)atof(val);
        else if (!strcmp(key, "shake_decay"))          t->shake_decay = (float)atof(val);
        else if (!strcmp(key, "shake_trauma_hit"))     t->shake_trauma_hit = (float)atof(val);
        else if (!strcmp(key, "shake_trauma_parry"))   t->shake_trauma_parry = (float)atof(val);
        else if (!strcmp(key, "shake_trauma_perfect")) t->shake_trauma_perfect = (float)atof(val);
        else if (!strcmp(key, "sprint_speed"))         t->sprint_speed = (float)atof(val);
        else if (!strcmp(key, "kenotita_regen"))       t->kenotita_regen = (float)atof(val);
        /* int keys */
        else if (!strcmp(key, "coyote_time"))          t->coyote_time = atoi(val);
        else if (!strcmp(key, "jump_buffer"))          t->jump_buffer = atoi(val);
        else if (!strcmp(key, "parry_freeze_ticks"))   t->parry_freeze_ticks = atoi(val);
        else if (!strcmp(key, "parry_perfect_window")) t->parry_perfect_window = atoi(val);
        else if (!strcmp(key, "parry_stun_ticks"))     t->parry_stun_ticks = atoi(val);
        else if (!strcmp(key, "hitstop_hit"))          t->hitstop_hit = atoi(val);
        else if (!strcmp(key, "hitstop_parry"))        t->hitstop_parry = atoi(val);
        else if (!strcmp(key, "hitstop_perfect"))      t->hitstop_perfect = atoi(val);
        else if (!strcmp(key, "hitstop_kill"))         t->hitstop_kill = atoi(val);
        else if (!strcmp(key, "sprint_duration_ticks"))t->sprint_duration_ticks = atoi(val);
        else if (!strcmp(key, "kenotita_sprint_cost")) t->kenotita_sprint_cost = atoi(val);
        else {
            fprintf(stderr, "[tunables] unknown key '%s'\n", key);
            ok = false;
        }
    }
    fclose(f);
    return ok;
}

bool tunables_reload(Tunables* t, const char* path) {
    Tunables tmp = *t;   /* snapshot in case parse fails */
    if (!tunables_load_file(&tmp, path)) return false;
    *t = tmp;
    return true;
}
