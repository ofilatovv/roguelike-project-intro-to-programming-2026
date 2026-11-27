// room.c - кімната: двері, вороги, предмети, масив 21x21
#include <stdlib.h>
#include "room.h"

// Зсуви для напрямків UP, RIGHT, DOWN, LEFT
const int DIR_DX[DIR_COUNT] = { 0, 1, 0, -1 };
const int DIR_DY[DIR_COUNT] = { -1, 0, 1, 0 };

Direction dir_opposite(Direction d) {
    return (Direction)((d + 2) % DIR_COUNT);
}

// Двері по центру стіни
void room_door_cell(Direction d, int *x, int *y) {
    *x = ROOM_CENTER + DIR_DX[d] * ROOM_CENTER;
    *y = ROOM_CENTER + DIR_DY[d] * ROOM_CENTER;
}

// Клітинка біля дверей, де з'являється гравець
void room_entry_cell(Direction d, int *x, int *y) {
    *x = ROOM_CENTER + DIR_DX[d] * (ROOM_CENTER - 1);
    *y = ROOM_CENTER + DIR_DY[d] * (ROOM_CENTER - 1);
}

void room_init(Room *r) {
    for (int d = 0; d < DIR_COUNT; d++) r->door_target[d] = -1;
    r->exit_dir = -1;
    r->enemy_count = 0;
    for (int i = 0; i < MAX_ITEMS; i++) {
        r->items[i].type = ITEM_NONE;
        r->items[i].x = r->items[i].y = 0;
    }
    r->reward_rolled = 1;
}

int room_alive_enemies(const Room *r) {
    int n = 0;
    for (int i = 0; i < r->enemy_count; i++)
        if (r->enemies[i].hp > 0) n++;
    return n;
}

int room_is_open(const Room *r) {
    return room_alive_enemies(r) == 0;
}

Entity *room_enemy_at(Room *r, int x, int y) {
    for (int i = 0; i < r->enemy_count; i++) {
        Entity *e = &r->enemies[i];
        if (e->hp > 0 && e->x == x && e->y == y) return e;
    }
    return NULL;
}

int room_door_dir_at(const Room *r, int x, int y) {
    for (int d = 0; d < DIR_COUNT; d++) {
        if (r->door_target[d] < 0) continue;
        int dx, dy;
        room_door_cell((Direction)d, &dx, &dy);
        if (dx == x && dy == y) return d;
    }
    return -1;
}

int room_exit_at(const Room *r, int x, int y) {
    if (r->exit_dir < 0) return 0;
    int ex, ey;
    room_door_cell((Direction)r->exit_dir, &ex, &ey);
    return ex == x && ey == y;
}

// Створення ворогів у випадкових клітинках
void room_spawn_enemies(Room *r, int floor_number) {
    int count = 1 + rand() % (3 + floor_number / 3);
    if (count > MAX_ENEMIES) count = MAX_ENEMIES;

    for (int i = 0; i < count; i++) {
        int x, y, clash;
        do {
            x = 3 + rand() % (ROOM_SIZE - 6);
            y = 3 + rand() % (ROOM_SIZE - 6);
            clash = 0;
            for (int j = 0; j < i; j++)
                if (r->enemies[j].x == x && r->enemies[j].y == y) clash = 1;
        } while (clash);

        r->enemies[i].x = x;
        r->enemies[i].y = y;
        r->enemies[i].hp = ENEMY_START_HP;
        r->enemies[i].max_hp = ENEMY_START_HP;
    }
    r->enemy_count = count;
    r->reward_rolled = 0;
}

Item *room_item_at(Room *r, int x, int y) {
    for (int i = 0; i < MAX_ITEMS; i++)
        if (r->items[i].type != ITEM_NONE && r->items[i].x == x && r->items[i].y == y)
            return &r->items[i];
    return NULL;
}

// Розміщення предмета в клітинці або в найближчій вільній
int room_place_item(Room *r, ItemType type, int cx, int cy, const Entity *player) {
    Item *slot = NULL;
    for (int i = 0; i < MAX_ITEMS; i++)
        if (r->items[i].type == ITEM_NONE) { slot = &r->items[i]; break; }
    if (!slot) return 0;

    for (int rad = 0; rad < ROOM_SIZE; rad++) {
        for (int dy = -rad; dy <= rad; dy++) {
            for (int dx = -rad; dx <= rad; dx++) {
                if (abs(dx) != rad && abs(dy) != rad) continue;
                int x = cx + dx, y = cy + dy;
                if (x < 1 || y < 1 || x > ROOM_SIZE - 2 || y > ROOM_SIZE - 2) continue;
                if (player->x == x && player->y == y) continue;
                if (room_item_at(r, x, y)) continue;
                slot->x = x;
                slot->y = y;
                slot->type = type;
                return 1;
            }
        }
    }
    return 0;
}

// Нагорода за зачистку: монета 25%, зілля 25% (50% при HP менше 3)
ItemType room_roll_reward(Room *r, const Entity *player) {
    r->reward_rolled = 1;

    int potion_chance = REWARD_POTION_CHANCE;
    if (player->hp < LOW_HP_THRESHOLD)
        potion_chance *= 2;

    int roll = rand() % 100;
    ItemType type;
    if (roll < REWARD_COIN_CHANCE)                       type = ITEM_COIN;
    else if (roll < REWARD_COIN_CHANCE + potion_chance)  type = ITEM_POTION;
    else                                                 return ITEM_NONE;

    return room_place_item(r, type, ROOM_CENTER, ROOM_CENTER, player) ? type : ITEM_NONE;
}

// Побудова масиву 21x21 шарами
void room_build_grid(const Room *r, const Entity *player, Tile grid[ROOM_SIZE][ROOM_SIZE]) {
    int open = room_is_open(r);

    // Підлога і стіни
    for (int y = 0; y < ROOM_SIZE; y++) {
        for (int x = 0; x < ROOM_SIZE; x++) {
            int border = (x == 0 || y == 0 || x == ROOM_SIZE - 1 || y == ROOM_SIZE - 1);
            grid[y][x].kind = border ? TILE_WALL : TILE_FLOOR;
            grid[y][x].open = 0;
        }
    }

    // Двері
    for (int d = 0; d < DIR_COUNT; d++) {
        if (r->door_target[d] < 0) continue;
        int x, y;
        room_door_cell((Direction)d, &x, &y);
        grid[y][x].kind = (d == DIR_UP || d == DIR_DOWN) ? TILE_DOOR_H : TILE_DOOR_V;
        grid[y][x].open = open;
    }

    // Вихід на наступний поверх
    if (r->exit_dir >= 0) {
        int x, y;
        room_door_cell((Direction)r->exit_dir, &x, &y);
        grid[y][x].kind = (r->exit_dir == DIR_RIGHT) ? TILE_EXIT_R
                        : (r->exit_dir == DIR_LEFT)  ? TILE_EXIT_L : TILE_EXIT_U;
        grid[y][x].open = open;
    }

    // Предмети
    for (int i = 0; i < MAX_ITEMS; i++)
        if (r->items[i].type != ITEM_NONE)
            grid[r->items[i].y][r->items[i].x].kind =
                (r->items[i].type == ITEM_COIN) ? TILE_COIN : TILE_POTION;

    // Вороги
    for (int i = 0; i < r->enemy_count; i++)
        if (r->enemies[i].hp > 0)
            grid[r->enemies[i].y][r->enemies[i].x].kind = TILE_ENEMY;

    // Гравець зверху
    grid[player->y][player->x].kind = TILE_PLAYER;
}
