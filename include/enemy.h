/* enemy.h — простий ШІ ворога: наближатись до гравця. */
#ifndef ENEMY_H
#define ENEMY_H
#include "types.h"

int  entity_is_adjacent(const Entity *a, const Entity *b);              /* клітинка до клітинки (без діагоналей) */
void enemy_step_toward(const Room *room, Entity *enemy, const Entity *player); /* один крок до гравця */

#endif
