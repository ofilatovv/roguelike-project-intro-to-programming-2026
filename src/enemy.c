/*
 * enemy.c — ШІ ворога. Після кожного кроку гравця ворог робить 1 крок до нього.
 */
#include <stdlib.h>
#include "enemy.h"
#include "room.h"

/* Сусідство "клітинка до клітинки": сума відстаней по осях дорівнює 1. */
int entity_is_adjacent(const Entity *a, const Entity *b) {
    return abs(a->x - b->x) + abs(a->y - b->y) == 1;
}

/* Чи може ворог стати у (x,y): всередині кімнати, не на гравці, не на іншому ворогові. */
static int cell_free_for_enemy(const Room *room, const Entity *self,
                               const Entity *player, int x, int y) {
    if (x <= 0 || x >= ROOM_SIZE - 1 || y <= 0 || y >= ROOM_SIZE - 1) return 0;
    if (player->x == x && player->y == y) return 0;
    for (int i = 0; i < room->enemy_count; i++) {
        const Entity *o = &room->enemies[i];
        if (o != self && o->hp > 0 && o->x == x && o->y == y) return 0;
    }
    return 1;
}

void enemy_step_toward(const Room *room, Entity *enemy, const Entity *player) {
    int dx = player->x - enemy->x;
    int dy = player->y - enemy->y;
    int sx = (dx > 0) - (dx < 0);   /* знак: -1, 0 або 1 */
    int sy = (dy > 0) - (dy < 0);

    /* Спершу крокуємо по осі з більшою відстанню, потім пробуємо іншу вісь. */
    int try_x_first = abs(dx) >= abs(dy);
    for (int pass = 0; pass < 2; pass++) {
        int use_x = (pass == 0) ? try_x_first : !try_x_first;
        int nx = enemy->x + (use_x ? sx : 0);
        int ny = enemy->y + (use_x ? 0 : sy);
        if ((use_x ? sx : sy) == 0) continue;  /* по цій осі вже збігаємось */
        if (cell_free_for_enemy(room, enemy, player, nx, ny)) {
            enemy->x = nx;
            enemy->y = ny;
            return;
        }
    }
    /* Обидва напрямки заблоковані — ворог стоїть. */
}
