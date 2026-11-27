// enemy.h - рух ворога
#ifndef ENEMY_H
#define ENEMY_H
#include "types.h"

int  entity_is_adjacent(const Entity *a, const Entity *b);
void enemy_step_toward(const Room *room, Entity *enemy, const Entity *player);

#endif
