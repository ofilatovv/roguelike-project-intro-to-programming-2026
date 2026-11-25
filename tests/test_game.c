/* test_game.c — правила ходу: стіни, бій, двері, нагороди, очки, смерть. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "room.h"
#include "stats.h"

/* Новий забіг без запису у файл. */
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

/* Кладе єдиний предмет у слот 0 (кімната в тестах попередньо порожня). */
static void put_item(Room *r, ItemType t, int x, int y) {
    r->items[0].type = t; r->items[0].x = x; r->items[0].y = y;
}

int main(void) {
    Game g;

    /* --- старт забігу --- */
    setup(&g, 1);
    assert(g.stats.runs_played == 1);
    assert(g.score == 0 && g.floor_number == 1);
    assert(g.player.hp == 5 && g.player.max_hp == 5);
    assert(g.player.x == ROOM_CENTER && g.player.y == ROOM_CENTER);
    assert(room_alive_enemies(game_room(&g)) == 0);

    /* --- стіна: гравець лишається на місці --- */
    g.player.x = 1; g.player.y = 1;
    game_move_player(&g, DIR_UP);
    assert(g.player.x == 1 && g.player.y == 1 && g.player.hp == 5);

    /* --- звичайний крок --- */
    game_move_player(&g, DIR_RIGHT);
    assert(g.player.x == 2 && g.player.y == 1);

    /* --- ворог іде до гравця --- */
    setup(&g, 2);
    put_enemy(game_room(&g), 10, 1, 3);
    g.player.x = 1; g.player.y = 1;
    game_move_player(&g, DIR_UP);                       /* даремний хід у стіну */
    assert(game_room(&g)->enemies[0].x == 9);

    /* --- стіна + ворог поруч = гарантований удар і штраф -20 --- */
    setup(&g, 3);
    put_enemy(game_room(&g), 2, 1, 3);
    g.player.x = 1; g.player.y = 1;
    g.score = 100;
    game_move_player(&g, DIR_UP);
    assert(g.player.hp == 4 && g.score == 80);

    /* --- смерть --- */
    g.player.hp = 1;
    game_move_player(&g, DIR_UP);
    assert(g.dead && g.player.hp == 0);

    /* --- вбивство ворога: +100, нагорода розіграна один раз --- */
    setup(&g, 4);
    put_enemy(game_room(&g), 2, 1, 1);
    g.player.x = 1; g.player.y = 1;
    game_move_player(&g, DIR_RIGHT);
    assert(room_alive_enemies(game_room(&g)) == 0);
    assert(g.score == 100 && g.player.hp == 5 && g.player.x == 1);
    assert(game_room(&g)->reward_rolled == 1);

    /* --- монета: +100 --- */
    setup(&g, 5);
    put_item(game_room(&g), ITEM_COIN, 2, 1);
    g.player.x = 1; g.player.y = 1;
    game_move_player(&g, DIR_RIGHT);
    assert(g.score == 100 && game_room(&g)->items[0].type == ITEM_NONE);
    assert(g.stats.max_score == 100);            /* рекорд оновився одразу */

    /* --- зілля: лікує, а при повному HP дає +300 очок --- */
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
    assert(g.player.hp == 5 && g.score == 300);  /* HP не стало 6 */

    /* --- max_score не зменшується, коли поточні очки нижчі --- */
    g.stats.max_score = 3200;
    g.score = 2000;
    game_add_score(&g, -20);
    assert(g.stats.max_score == 3200 && g.score == 1980);

    /* --- двері: відкриті пропускають, зачинені — ні --- */
    setup(&g, 7);
    Room *start = game_room(&g);
    int d = -1;
    for (int i = 0; i < DIR_COUNT; i++) if (start->door_target[i] >= 0) { d = i; break; }
    assert(d >= 0);
    int target = start->door_target[d];
    int ex, ey;

    put_enemy(start, 5, 5, 3);                    /* ворог -> двері зачинені */
    room_entry_cell((Direction)d, &ex, &ey);
    g.player.x = ex; g.player.y = ey;
    game_move_player(&g, (Direction)d);
    assert(g.current_room == g.floor.start_room); /* не пройшли */

    start->enemies[0].hp = 0;                     /* ворога немає -> двері відкриті */
    g.player.x = ex; g.player.y = ey;
    game_move_player(&g, (Direction)d);
    assert(g.current_room == target);
    room_entry_cell(dir_opposite((Direction)d), &ex, &ey);
    assert(g.player.x == ex && g.player.y == ey); /* з'явились біля протилежних дверей */

    /* --- вихід на наступний поверх: +500 і порожня стартова кімната --- */
    setup(&g, 8);
    start = game_room(&g);
    start->door_target[DIR_RIGHT] = -1;           /* звільняємо праву стіну під вихід */
    start->exit_dir = DIR_RIGHT;
    g.player.x = ROOM_SIZE - 2; g.player.y = ROOM_CENTER;
    game_move_player(&g, DIR_RIGHT);
    assert(g.floor_number == 2 && g.score == SCORE_NEXT_FLOOR);
    assert(g.player.x == ROOM_CENTER && g.player.y == ROOM_CENTER);
    assert(room_alive_enemies(game_room(&g)) == 0);

    /* --- вбитий ворог залишає зілля під собою у ~50% випадків --- */
    int drops = 0, N = 2000;
    for (int i = 0; i < N; i++) {
        setup(&g, 1000 + (unsigned)i);
        put_enemy(game_room(&g), 2, 1, 1);
        g.player.x = 1; g.player.y = 1;
        game_move_player(&g, DIR_RIGHT);
        Item *it = room_item_at(game_room(&g), 2, 1);   /* саме клітинка, де стояв ворог */
        if (it && it->type == ITEM_POTION) drops++;
    }
    assert(drops > N * 45 / 100 && drops < N * 55 / 100);

    /* --- дроп не затирає гравця: якщо клітинка зайнята предметом, зілля лягає поруч --- */
    {
        Room r; room_init(&r);
        Entity p = { 1, 1, 5, 5 };
        assert(room_place_item(&r, ITEM_COIN, 5, 5, &p) == 1);
        assert(room_place_item(&r, ITEM_POTION, 5, 5, &p) == 1);   /* (5,5) зайнято -> сусідня */
        assert(room_item_at(&r, 5, 5)->type == ITEM_COIN);
        int potions = 0;
        for (int i = 0; i < MAX_ITEMS; i++) if (r.items[i].type == ITEM_POTION) potions++;
        assert(potions == 1);
    }

    printf("test_game: OK\n");
    return 0;
}
