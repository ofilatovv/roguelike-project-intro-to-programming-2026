// test_game.c - тест правил ходу
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "room.h"
#include "stats.h"

static void setup(Game *g, unsigned seed) {
    srand(seed);
    Stats s;
    stats_defaults(&s);
    game_init(g, &s, NULL);
    game_start_run(g);
}

static void put_enemy(Room *r, int x, int y, int hp) {
    Entity e = { x, y, hp, ENEMY_START_HP };
    r->enemies[0] = e;
    r->enemy_count = 1;
    r->reward_rolled = 0;
}

static void put_item(Room *r, ItemType t, int x, int y) {
    r->items[0].type = t; r->items[0].x = x; r->items[0].y = y;
}

int main(void) {
    Game g;

    // Старт забігу, стіна, крок
    setup(&g, 1);
    assert(g.stats.runs_played == 1);
    assert(g.score == 0 && g.floor_number == 1);
    assert(g.player.hp == 5 && g.player.max_hp == 5);
    assert(g.player.x == ROOM_CENTER && g.player.y == ROOM_CENTER);
    assert(room_alive_enemies(game_room(&g)) == 0);

    g.player.x = 1; g.player.y = 1;
    game_move_player(&g, DIR_UP);
    assert(g.player.x == 1 && g.player.y == 1 && g.player.hp == 5);

    game_move_player(&g, DIR_RIGHT);
    assert(g.player.x == 2 && g.player.y == 1);

    // Ворог іде до гравця
    setup(&g, 2);
    put_enemy(game_room(&g), 10, 1, 3);
    g.player.x = 1; g.player.y = 1;
    game_move_player(&g, DIR_UP);
    assert(game_room(&g)->enemies[0].x == 9);

    // Удар при даремному ході, смерть
    setup(&g, 3);
    put_enemy(game_room(&g), 2, 1, 3);
    g.player.x = 1; g.player.y = 1;
    g.score = 100;
    game_move_player(&g, DIR_UP);
    assert(g.player.hp == 4 && g.score == 80);

    g.player.hp = 1;
    game_move_player(&g, DIR_UP);
    assert(g.dead && g.player.hp == 0);

    // Вбивство ворога
    setup(&g, 4);
    put_enemy(game_room(&g), 2, 1, 1);
    g.player.x = 1; g.player.y = 1;
    game_move_player(&g, DIR_RIGHT);
    assert(room_alive_enemies(game_room(&g)) == 0);
    assert(g.score == 100 && g.player.hp == 5 && g.player.x == 1);
    assert(game_room(&g)->reward_rolled == 1);

    // Монета
    setup(&g, 5);
    put_item(game_room(&g), ITEM_COIN, 2, 1);
    g.player.x = 1; g.player.y = 1;
    game_move_player(&g, DIR_RIGHT);
    assert(g.score == 100 && game_room(&g)->items[0].type == ITEM_NONE);
    assert(g.stats.max_score == 100);

    // Зілля
    setup(&g, 6);
    put_item(game_room(&g), ITEM_POTION, 2, 1);
    g.player.x = 1; g.player.y = 1; g.player.hp = 3;
    game_move_player(&g, DIR_RIGHT);
    assert(g.player.hp == 4 && g.score == 0);

    put_item(game_room(&g), ITEM_POTION, 3, 1);
    game_move_player(&g, DIR_RIGHT);
    assert(g.player.hp == 5 && g.score == 0);

    put_item(game_room(&g), ITEM_POTION, 4, 1);
    game_move_player(&g, DIR_RIGHT);
    assert(g.player.hp == 5 && g.score == 300);

    // max_score не зменшується
    g.stats.max_score = 3200;
    g.score = 2000;
    game_add_score(&g, -20);
    assert(g.stats.max_score == 3200 && g.score == 1980);

    // Двері
    setup(&g, 7);
    Room *start = game_room(&g);
    int d = -1;
    for (int i = 0; i < DIR_COUNT; i++) if (start->door_target[i] >= 0) { d = i; break; }
    assert(d >= 0);
    int target = start->door_target[d];
    int ex, ey;

    put_enemy(start, 5, 5, 3);
    room_entry_cell((Direction)d, &ex, &ey);
    g.player.x = ex; g.player.y = ey;
    game_move_player(&g, (Direction)d);
    assert(g.current_room == g.floor.start_room);

    start->enemies[0].hp = 0;
    g.player.x = ex; g.player.y = ey;
    game_move_player(&g, (Direction)d);
    assert(g.current_room == target);
    room_entry_cell(dir_opposite((Direction)d), &ex, &ey);
    assert(g.player.x == ex && g.player.y == ey);

    // Вихід на наступний поверх
    setup(&g, 8);
    start = game_room(&g);
    start->door_target[DIR_RIGHT] = -1;
    start->exit_dir = DIR_RIGHT;
    g.player.x = ROOM_SIZE - 2; g.player.y = ROOM_CENTER;
    game_move_player(&g, DIR_RIGHT);
    assert(g.floor_number == 2 && g.score == SCORE_NEXT_FLOOR);
    assert(g.player.x == ROOM_CENTER && g.player.y == ROOM_CENTER);
    assert(room_alive_enemies(game_room(&g)) == 0);

    // Дроп зілля з ворога (статистика)
    int drops = 0, N = 2000;
    for (int i = 0; i < N; i++) {
        setup(&g, 1000 + (unsigned)i);
        put_enemy(game_room(&g), 2, 1, 1);
        g.player.x = 1; g.player.y = 1;
        game_move_player(&g, DIR_RIGHT);
        Item *it = room_item_at(game_room(&g), 2, 1);
        if (it && it->type == ITEM_POTION) drops++;
    }
    assert(drops > N * 45 / 100 && drops < N * 55 / 100);

    {
        Room r; room_init(&r);
        Entity p = { 1, 1, 5, 5 };
        assert(room_place_item(&r, ITEM_COIN, 5, 5, &p) == 1);
        assert(room_place_item(&r, ITEM_POTION, 5, 5, &p) == 1);
        assert(room_item_at(&r, 5, 5)->type == ITEM_COIN);
        int potions = 0;
        for (int i = 0; i < MAX_ITEMS; i++) if (r.items[i].type == ITEM_POTION) potions++;
        assert(potions == 1);
    }

    printf("test_game: OK\n");
    return 0;
}
