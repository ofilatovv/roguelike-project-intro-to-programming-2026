// test_minimap.c - тест мінімапи
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "floor.h"
#include "minimap.h"

static int count_kind(MiniCell m[MINIMAP_MAX][MINIMAP_MAX], int w, int h, MiniKind k) {
    int n = 0;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            if (m[y][x].kind == k) n++;
    return n;
}

int main(void) {
    // Перевірка 100 випадкових поверхів
    for (unsigned seed = 1; seed <= 100; seed++) {
        srand(seed);
        Floor f;
        floor_generate(&f, 1);

        MiniCell m[MINIMAP_MAX][MINIMAP_MAX];
        int w, h;
        minimap_build(&f, f.start_room, m, &w, &h);
        assert(w >= 1 && w <= MINIMAP_MAX && h >= 1 && h <= MINIMAP_MAX);

        int rooms = count_kind(m, w, h, MINI_ROOM_EMPTY) + count_kind(m, w, h, MINI_ROOM_FILLED);
        assert(rooms == f.room_count);

        assert(count_kind(m, w, h, MINI_ROOM_EMPTY) == f.room_count - 1);
        assert(count_kind(m, w, h, MINI_ROOM_FILLED) == 1);

        int links = count_kind(m, w, h, MINI_LINK_H) + count_kind(m, w, h, MINI_LINK_V);
        assert(links == f.room_count - 1);

        int cx, cy, cur = 0;
        for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) cur += m[y][x].current;
        assert(cur == 1);
        minimap_cell_of(&f, f.start_room, &cx, &cy);
        assert(m[cy][cx].current == 1 && m[cy][cx].kind == MINI_ROOM_FILLED);

        // Зачищена кімната стає заповненою
        int other = (f.start_room + 1) % f.room_count;
        for (int i = 0; i < f.rooms[other].enemy_count; i++) f.rooms[other].enemies[i].hp = 0;
        minimap_build(&f, other, m, &w, &h);
        minimap_cell_of(&f, other, &cx, &cy);
        assert(m[cy][cx].kind == MINI_ROOM_FILLED && m[cy][cx].current == 1);
        assert(count_kind(m, w, h, MINI_ROOM_FILLED) == 2);
        minimap_cell_of(&f, f.start_room, &cx, &cy);
        assert(m[cy][cx].current == 0);
    }
    printf("test_minimap: OK\n");
    return 0;
}
