#ifndef TV_COMBAT_H
#define TV_COMBAT_H

#include "player.h"
#include "enemy.h"

/*
 * Combat resolution. Drives:
 *   - parry clash detection (player attack vs enemy attack hitboxes)
 *   - perfect-parry window (first PARRY_PERFECT_WINDOW ticks of freeze)
 *   - normal hit detection (player attack vs enemy body)
 *   - enemy attack hitting player body
 *   - hit-stop, screen shake, particle bursts, audio cues
 *
 * All enemy interactions go through the vtable.
 *
 * Returns event flags via the World's effects struct (in world.h).
 */

void combat_resolve(Player* p, AttackHitbox* p_atk, Enemy* enemies, int n,
                    struct World* w);

#endif /* TV_COMBAT_H */
