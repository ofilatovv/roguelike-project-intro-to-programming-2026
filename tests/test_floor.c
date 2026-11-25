/* test_floor.c — інваріанти випадкової генерації поверху (200 різних seed). */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "floor.h"
#include "room.h"

int main(void) {
    for (unsigned seed = 1; seed <= 200; seed++) {
        srand(seed);
        Floor f;
        floor_generate(&f, 1 + (int)(seed % 10));

        assert(f.room_count >= 2 && f.room_count <= MAX_ROOMS);

        int exits = 0;
        for (int i = 0; i < f.room_count; i++) {
            Room *r = &f.rooms[i];

            /* 1..3 двері у кожній кімнаті */
            int doors = 0;
            for (int d = 0; d < DIR_COUNT; d++) {
                int t = r->door_target[d];
                if (t < 0) continue;
                doors++;
                /* двері взаємні: з іншого боку протилежні двері ведуть назад */
                assert(t < f.room_count);
                assert(f.rooms[t].door_target[dir_opposite((Direction)d)] == i);
            }
            assert(doors >= 1 && doors <= MAX_DOORS);

            /* стартова кімната без ворогів, решта — з ворогами */
            if (i == f.start_room) assert(room_alive_enemies(r) == 0);
            else assert(room_alive_enemies(r) >= 1 && room_alive_enemies(r) <= MAX_ENEMIES);

            if (r->exit_dir >= 0) {
                exits++;
                assert(i != f.start_room);            /* вихід не в стартовій кімнаті */
                assert(r->exit_dir != DIR_DOWN);      /* на нижній стіні виходу немає */
                assert(r->door_target[r->exit_dir] == -1); /* стіна не зайнята дверима */
            }
        }
        assert(exits == 1);
    }

    /* Масив-екран: стіни по краях, гравець у центрі, двері на місці. */
    srand(7);
    Floor f;
    floor_generate(&f, 1);
    Entity p = { ROOM_CENTER, ROOM_CENTER, 5, 5 };
    Tile grid[ROOM_SIZE][ROOM_SIZE];
    room_build_grid(&f.rooms[f.start_room], &p, grid);
    assert(grid[ROOM_CENTER][ROOM_CENTER].kind == TILE_PLAYER);
    assert(grid[1][1].kind == TILE_FLOOR);
    assert(grid[0][0].kind == TILE_WALL && grid[20][20].kind == TILE_WALL);
    for (int d = 0; d < DIR_COUNT; d++) {
        if (f.rooms[f.start_room].door_target[d] < 0) continue;
        int x, y;
        room_door_cell((Direction)d, &x, &y);
        char want = (d == DIR_UP || d == DIR_DOWN) ? TILE_DOOR_H : TILE_DOOR_V;
        assert(grid[y][x].kind == want && grid[y][x].open == 1); /* без ворогів двері відкриті */
    }

    printf("test_floor: OK\n");
    return 0;
}
