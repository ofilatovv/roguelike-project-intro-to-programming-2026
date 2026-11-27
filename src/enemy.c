// enemy.c - рух ворога до гравця
#include <stdlib.h>
#include "enemy.h"
#include "room.h"

// Сусідні клітинки без діагоналей
int entity_is_adjacent(const Entity *a, const Entity *b) {
    return abs(a->x - b->x) + abs(a->y - b->y) == 1;
}

// Перевірка, чи вільна клітинка для ворога
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

// Один крок: спершу по осі з більшою відстанню, потім по іншій
void enemy_step_toward(const Room *room, Entity *enemy, const Entity *player) {
    int dx = player->x - enemy->x;
    int dy = player->y - enemy->y;
    int sx = (dx > 0) - (dx < 0);
    int sy = (dy > 0) - (dy < 0);

    int try_x_first = abs(dx) >= abs(dy);
    for (int pass = 0; pass < 2; pass++) {
        int use_x = (pass == 0) ? try_x_first : !try_x_first;
        int nx = enemy->x + (use_x ? sx : 0);
        int ny = enemy->y + (use_x ? 0 : sy);
        if ((use_x ? sx : sy) == 0) continue;
        if (cell_free_for_enemy(room, enemy, player, nx, ny)) {
            enemy->x = nx;
            enemy->y = ny;
            return;
        }
    }
}
