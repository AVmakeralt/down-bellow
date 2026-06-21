#ifndef TV_COMBAT_H
#define TV_COMBAT_H

#include "player.h"
#include "enemy.h"

/*
 * Combat resolution.
 *
 * Each tick we check:
 *   1. Player's voidslash hitbox vs each enemy's body box. If hit and
 *      enemy isn't already in PARRY-frozen state, deal damage (unless
 *      enemy is mid-attack and the attack is parryable -> start parry).
 *   2. Player's hitbox vs enemy's attack hitbox -> start a parry clash.
 *   3. Enemy's attack hitbox vs player's body box -> player takes damage
 *      (unless player is mid-i-frames).
 */

void combat_resolve(Player* p, AttackHitbox* p_atk, Enemy* enemies, int n);

#endif /* TV_COMBAT_H */
