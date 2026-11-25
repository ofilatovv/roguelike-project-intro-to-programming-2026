/*
 * floor.c — випадкова генерація поверху.
 *
 * Алгоритм: кімнати "виростають" на сітці від стартової. Кожна нова кімната
 * чіпляється дверима до випадкової існуючої сусідньою клітинкою сітки.
 * Виходить зв'язне дерево: кожна кімната має мінімум 1 двері, максимум 3.
 */
#include <stdlib.h>
#include <string.h>
#include "floor.h"
#include "room.h"

#define GRID (2 * MAX_ROOMS + 1)   /* сітка з запасом, щоб не вийти за межі */

static int add_room(Floor *f, int map[GRID][GRID], int gx, int gy) {
    int id = f->room_count++;
    room_init(&f->rooms[id]);
    f->rooms[id].grid_x = gx;
    f->rooms[id].grid_y = gy;
    map[gy][gx] = id;
    return id;
}

static int door_count(const Room *r) {
    int n = 0;
    for (int d = 0; d < DIR_COUNT; d++)
        if (r->door_target[d] >= 0) n++;
    return n;
}

void floor_generate(Floor *f, int floor_number) {
    int map[GRID][GRID];
    for (int y = 0; y < GRID; y++)
        for (int x = 0; x < GRID; x++) map[y][x] = -1;

    memset(f, 0, sizeof *f);
    f->room_count = 0;

    /* Стартова кімната — у центрі сітки. */
    f->start_room = add_room(f, map, GRID / 2, GRID / 2);

    /* ---- 1. Нарощуємо кімнати ---- */
    int target = MIN_ROOMS + rand() % (MAX_ROOMS - MIN_ROOMS + 1);   /* 5..7 */
    int attempts = 0;
    while (f->room_count < target && attempts++ < 5000) {
        int parent = rand() % f->room_count;
        Direction d = (Direction)(rand() % DIR_COUNT);
        Room *p = &f->rooms[parent];

        if (p->door_target[d] != -1 || door_count(p) >= MAX_DOORS) continue;

        int nx = p->grid_x + DIR_DX[d];
        int ny = p->grid_y + DIR_DY[d];
        if (nx < 0 || ny < 0 || nx >= GRID || ny >= GRID) continue;
        if (map[ny][nx] != -1) continue;                 /* клітинка вже зайнята */

        int id = add_room(f, map, nx, ny);
        p->door_target[d] = id;                          /* двері туди ... */
        f->rooms[id].door_target[dir_opposite(d)] = parent; /* ... і двері назад */
    }

    /* ---- 2. Відстані від старту (BFS по дверях) ---- */
    int dist[MAX_ROOMS], queue[MAX_ROOMS], qh = 0, qt = 0;
    for (int i = 0; i < MAX_ROOMS; i++) dist[i] = -1;
    dist[f->start_room] = 0;
    queue[qt++] = f->start_room;
    while (qh < qt) {
        int r = queue[qh++];
        for (int d = 0; d < DIR_COUNT; d++) {
            int t = f->rooms[r].door_target[d];
            if (t >= 0 && dist[t] < 0) {
                dist[t] = dist[r] + 1;
                queue[qt++] = t;
            }
        }
    }

    /* ---- 3. Вихід: у найдальшій кімнаті, на вільній стіні (не низ) ---- */
    static const Direction exit_dirs[3] = { DIR_UP, DIR_RIGHT, DIR_LEFT };
    int maxd = 0;
    for (int i = 0; i < f->room_count; i++)
        if (dist[i] > maxd) maxd = dist[i];

    for (int dd = maxd; dd >= 1; dd--) {
        int cand_room[MAX_ROOMS * 3], cand_dir[MAX_ROOMS * 3], n = 0;
        for (int i = 0; i < f->room_count; i++) {
            if (dist[i] != dd) continue;
            for (int k = 0; k < 3; k++)
                if (f->rooms[i].door_target[exit_dirs[k]] == -1) {   /* стіна вільна від дверей */
                    cand_room[n] = i;
                    cand_dir[n] = exit_dirs[k];
                    n++;
                }
        }
        if (n > 0) {
            int pick = rand() % n;
            f->rooms[cand_room[pick]].exit_dir = cand_dir[pick];
            break;
        }
    }

    /* ---- 4. Вороги у всіх кімнатах, крім стартової ---- */
    for (int i = 0; i < f->room_count; i++)
        if (i != f->start_room) room_spawn_enemies(&f->rooms[i], floor_number);
}
