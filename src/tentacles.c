#include "tentacles.h"
#include "render.h"
#include "camera.h"
#include "config.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void tentacles_init(TentacleRig* rig) {
    memset(rig, 0, sizeof(*rig));
    rig->count = 3;
    for (int i = 0; i < 4; i++) {
        rig->t[i].phase = (float)i * 0.7f;
        rig->t[i].side  = (i % 2 == 0) ? -1 : +1;
        for (int s = 0; s < TENTACLE_SEGMENTS; s++) {
            rig->t[i].segments[s] = vec2(0, 0);
        }
    }
}

void tentacles_update(TentacleRig* rig, const Player* p, float dt) {
    rig->elapsed += dt;
    /* head anchor: torso center of the player */
    Vec2 head = vec2(p->pos.x + PLAYER_W * 0.5f,
                     p->pos.y + PLAYER_H * 0.5f);

    for (int i = 0; i < rig->count; i++) {
        Tentacle* t = &rig->t[i];
        /* attach head */
        t->segments[0] = head;
        /* thrash: each segment sways with sine offset */
        for (int s = 1; s < TENTACLE_SEGMENTS; s++) {
            float time = rig->elapsed * 18.0f + (float)s * 1.3f + t->phase;
            float sway_y = sinf(time) * (float)s * 1.2f;
            float sway_x = cosf(time * 0.7f) * (float)s * 0.6f;
            /* anchor: pull OPPOSITE to facing (tentacles trail behind) */
            Vec2 target;
            target.x = t->segments[s-1].x - p->facing * TENTACLE_SEGMENT_LEN + sway_x * t->side;
            target.y = t->segments[s-1].y + sway_y + (float)s * 0.8f;
            /* ease toward target */
            float k = 0.45f;
            t->segments[s].x += (target.x - t->segments[s].x) * k;
            t->segments[s].y += (target.y - t->segments[s].y) * k;
            /* enforce distance constraint (project back to radius) */
            float dx = t->segments[s].x - t->segments[s-1].x;
            float dy = t->segments[s].y - t->segments[s-1].y;
            float d = sqrtf(dx*dx + dy*dy);
            if (d > 0.0001f) {
                float want = TENTACLE_SEGMENT_LEN;
                float scale = want / d;
                t->segments[s].x = t->segments[s-1].x + dx * scale;
                t->segments[s].y = t->segments[s-1].y + dy * scale;
            }
        }
    }
}

void tentacles_draw(const TentacleRig* rig, Renderer* r, const Camera* c) {
    /* draw as decaying black circles per segment */
    for (int i = 0; i < rig->count; i++) {
        const Tentacle* t = &rig->t[i];
        for (int s = 0; s < TENTACLE_SEGMENTS; s++) {
            IVec2 sp = camera_world_to_screen(c, t->segments[s].x, t->segments[s].y);
            int size = 5 - s;   /* 4,3,2,1,1 */
            if (size < 1) size = 1;
            IRect rc = { sp.x - size, sp.y - size, size * 2, size * 2 };
            /* darker = closer to head; lighter = tip */
            Uint8 a = (Uint8)(220 - s * 30);
            Color col = 0x00000000 | ((uint32_t)a << 24);
            draw_rect_screen(r, rc, col, 1);
        }
    }
}
