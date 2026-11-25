/*
 * room.c — усе, що стосується однієї кімнати:
 * координати дверей, вороги, нагорода та побудова двовимірного масиву 21x21.
 */
#include <stdlib.h>
#include "room.h"

/* Зсуви для напрямків: UP, RIGHT, DOWN, LEFT. */
const int DIR_DX[DIR_COUNT] = { 0, 1, 0, -1 };
const int DIR_DY[DIR_COUNT] = { -1, 0, 1, 0 };

Direction dir_opposite(Direction d) {
    return (Direction)((d + 2) % DIR_COUNT);   /* UP<->DOWN, LEFT<->RIGHT */
}

/* Двері лежать рівно по центру стіни: центр + напрямок * 10. */
void room_door_cell(Direction d, int *x, int *y) {
    *x = ROOM_CENTER + DIR_DX[d] * ROOM_CENTER;
    *y = ROOM_CENTER + DIR_DY[d] * ROOM_CENTER;
}

/* Клітинка, на якій з'являється гравець, зайшовши в кімнату через ці двері. */
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
    r->reward_rolled = 1;   /* без ворогів нагороди за зачистку немає */
}

int room_alive_enemies(const Room *r) {
    int n = 0;
    for (int i = 0; i < r->enemy_count; i++)
        if (r->enemies[i].hp > 0) n++;
    return n;
}

/* Двері й вихід відчинені тоді й лише тоді, коли в кімнаті не лишилось ворогів. */
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

/* Створює ворогів. Їх кількість трохи зростає з номером поверху. */
void room_spawn_enemies(Room *r, int floor_number) {
    int count = 1 + rand() % (3 + floor_number / 3);
    if (count > MAX_ENEMIES) count = MAX_ENEMIES;

    for (int i = 0; i < count; i++) {
        int x, y, clash;
        do {
            /* Діапазон 3..17: подалі від клітинок біля дверей, де з'являється гравець. */
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
    r->reward_rolled = 0;   /* нагороду розіграємо, коли всіх уб'ють */
}

Item *room_item_at(Room *r, int x, int y) {
    for (int i = 0; i < MAX_ITEMS; i++)
        if (r->items[i].type != ITEM_NONE && r->items[i].x == x && r->items[i].y == y)
            return &r->items[i];
    return NULL;
}

/*
 * Кладе предмет у клітинку (cx,cy). Якщо вона зайнята (гравцем чи іншим
 * предметом) — шукає найближчу вільну кільцями навколо: радіус 0, 1, 2, ...
 */
int room_place_item(Room *r, ItemType type, int cx, int cy, const Entity *player) {
    Item *slot = NULL;
    for (int i = 0; i < MAX_ITEMS; i++)
        if (r->items[i].type == ITEM_NONE) { slot = &r->items[i]; break; }
    if (!slot) return 0;

    for (int rad = 0; rad < ROOM_SIZE; rad++) {
        for (int dy = -rad; dy <= rad; dy++) {
            for (int dx = -rad; dx <= rad; dx++) {
                if (abs(dx) != rad && abs(dy) != rad) continue;   /* лише межа кільця */
                int x = cx + dx, y = cy + dy;
                if (x < 1 || y < 1 || x > ROOM_SIZE - 2 || y > ROOM_SIZE - 2) continue; /* не в стіну */
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

/*
 * Розіграш нагороди після зачистки. Один кидок 0..99:
 *   монета: 0..24 (завжди 25%)
 *   зілля : наступні 25% — або 50%, якщо HP гравця строго менше 3.
 * Нагорода кладеться в центр кімнати (або поруч, якщо центр зайнятий).
 */
ItemType room_roll_reward(Room *r, const Entity *player) {
    r->reward_rolled = 1;

    int potion_chance = REWARD_POTION_CHANCE;
    if (player->hp < LOW_HP_THRESHOLD)      /* саме "<", а не "<=": при HP == 3 бонусу немає */
        potion_chance *= 2;

    int roll = rand() % 100;
    ItemType type;
    if (roll < REWARD_COIN_CHANCE)                       type = ITEM_COIN;
    else if (roll < REWARD_COIN_CHANCE + potion_chance)  type = ITEM_POTION;
    else                                                 return ITEM_NONE;

    return room_place_item(r, type, ROOM_CENTER, ROOM_CENTER, player) ? type : ITEM_NONE;
}

/*
 * Будує двовимірний масив-екран. Шари малюються знизу вгору:
 * підлога/стіни -> двері/вихід -> предмет -> вороги -> гравець.
 */
void room_build_grid(const Room *r, const Entity *player, Tile grid[ROOM_SIZE][ROOM_SIZE]) {
    int open = room_is_open(r);

    /* 1. Підлога всередині, стіни по периметру. */
    for (int y = 0; y < ROOM_SIZE; y++) {
        for (int x = 0; x < ROOM_SIZE; x++) {
            int border = (x == 0 || y == 0 || x == ROOM_SIZE - 1 || y == ROOM_SIZE - 1);
            grid[y][x].kind = border ? TILE_WALL : TILE_FLOOR;
            grid[y][x].open = 0;
        }
    }

    /* 2. Двері: на верхній/нижній стіні "—", на лівій/правій "|". */
    for (int d = 0; d < DIR_COUNT; d++) {
        if (r->door_target[d] < 0) continue;
        int x, y;
        room_door_cell((Direction)d, &x, &y);
        grid[y][x].kind = (d == DIR_UP || d == DIR_DOWN) ? TILE_DOOR_H : TILE_DOOR_V;
        grid[y][x].open = open;
    }

    /* 3. Вихід на наступний поверх: ">" праворуч, "<" ліворуч, "^" угорі. */
    if (r->exit_dir >= 0) {
        int x, y;
        room_door_cell((Direction)r->exit_dir, &x, &y);
        grid[y][x].kind = (r->exit_dir == DIR_RIGHT) ? TILE_EXIT_R
                        : (r->exit_dir == DIR_LEFT)  ? TILE_EXIT_L : TILE_EXIT_U;
        grid[y][x].open = open;
    }

    /* 4. Нагорода. */
    for (int i = 0; i < MAX_ITEMS; i++)
        if (r->items[i].type != ITEM_NONE)
            grid[r->items[i].y][r->items[i].x].kind =
                (r->items[i].type == ITEM_COIN) ? TILE_COIN : TILE_POTION;

    /* 5. Живі вороги. */
    for (int i = 0; i < r->enemy_count; i++)
        if (r->enemies[i].hp > 0)
            grid[r->enemies[i].y][r->enemies[i].x].kind = TILE_ENEMY;

    /* 6. Гравець завжди зверху. */
    grid[player->y][player->x].kind = TILE_PLAYER;
}
