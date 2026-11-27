// game.c - правила ходу, очки, смерть
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "combat.h"
#include "enemy.h"
#include "floor.h"
#include "room.h"
#include "stats.h"

static void set_message(Game *g, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g->message, MSG_LEN, fmt, ap);
    va_end(ap);
}

static void append_message(Game *g, const char *fmt, ...) {
    size_t len = strlen(g->message);
    if (len >= MSG_LEN - 1) return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g->message + len, MSG_LEN - len, fmt, ap);
    va_end(ap);
}

Room *game_room(Game *g) {
    return &g->floor.rooms[g->current_room];
}

void game_init(Game *g, const Stats *stats, const char *stats_path) {
    g->stats = *stats;
    g->stats_path = stats_path;
    g->score = 0;
    g->dead = 0;
    g->message[0] = '\0';
}

// Зміна очок з оновленням max_score
void game_add_score(Game *g, int delta) {
    g->score += delta;
    if (g->score < 0) g->score = 0;
    if (g->score > g->stats.max_score) {
        g->stats.max_score = g->score;
        if (g->stats_path) stats_save(&g->stats, g->stats_path);
    }
}

// Новий поверх, гравець у центрі стартової кімнати
static void enter_floor(Game *g, int floor_number) {
    g->floor_number = floor_number;
    floor_generate(&g->floor, floor_number);
    g->current_room = g->floor.start_room;
    g->player.x = ROOM_CENTER;
    g->player.y = ROOM_CENTER;
}

// Початок забігу, runs_played + 1
void game_start_run(Game *g) {
    g->stats.runs_played++;
    if (g->stats_path) stats_save(&g->stats, g->stats_path);

    g->score = 0;
    g->dead = 0;
    g->player.hp = PLAYER_MAX_HP;
    g->player.max_hp = PLAYER_MAX_HP;
    enter_floor(g, 1);
    set_message(g, "Find the purple exit!");
}

// Штраф -20 очок за кожне втрачене HP, перевірка смерті
static void apply_hp_loss(Game *g, int lost) {
    if (lost > 0) game_add_score(g, -SCORE_HP_LOSS * lost);
    if (g->player.hp <= 0) {
        g->player.hp = 0;
        g->dead = 1;
        set_message(g, "You died!");
    }
}

// Нагорода після зачистки кімнати
static void check_room_cleared(Game *g) {
    Room *room = game_room(g);
    if (room_alive_enemies(room) == 0 && !room->reward_rolled) {
        ItemType t = room_roll_reward(room, &g->player);
        append_message(g, " Room cleared!");
        if (t == ITEM_COIN)        append_message(g, " A coin appeared.");
        else if (t == ITEM_POTION) append_message(g, " A potion appeared.");
    }
}

// Підбір предмета
static void pickup_item(Game *g) {
    Room *room = game_room(g);
    Item *it = room_item_at(room, g->player.x, g->player.y);
    if (!it) return;

    if (it->type == ITEM_COIN) {
        game_add_score(g, SCORE_COIN);
        set_message(g, "+%d points (coin)", SCORE_COIN);
    } else {
        if (g->player.hp < g->player.max_hp) {
            g->player.hp++;
            set_message(g, "+1 HP (potion)");
        } else {
            game_add_score(g, SCORE_POTION_FULL);
            set_message(g, "+%d points (potion at full HP)", SCORE_POTION_FULL);
        }
    }
    it->type = ITEM_NONE;
}

// Хід ворогів: удар при даремному ході гравця, інакше рух до гравця
static void enemies_turn(Game *g, int wasted) {
    Room *room = game_room(g);
    for (int i = 0; i < room->enemy_count && !g->dead; i++) {
        Entity *e = &room->enemies[i];
        if (e->hp <= 0) continue;

        if (entity_is_adjacent(e, &g->player)) {
            if (wasted) {
                g->player.hp -= 1;
                apply_hp_loss(g, 1);
                if (!g->dead) set_message(g, "An enemy hits you while you stumble!");
            }

        } else {
            enemy_step_toward(room, e, &g->player);
        }
    }
}

// Перехід у сусідню кімнату
static void use_door(Game *g, Direction d) {
    Room *cur = game_room(g);
    g->current_room = cur->door_target[d];

    int ex, ey;
    room_entry_cell(dir_opposite(d), &ex, &ey);
    g->player.x = ex;
    g->player.y = ey;

    if (room_alive_enemies(game_room(g)) > 0) set_message(g, "The doors slam shut behind you!");
    else                                       set_message(g, "This room is quiet.");
}

// Перехід на наступний поверх
static void use_exit(Game *g) {
    game_add_score(g, SCORE_NEXT_FLOOR);
    enter_floor(g, g->floor_number + 1);
    set_message(g, "Descended to floor %d! (+%d)", g->floor_number, SCORE_NEXT_FLOOR);
}

// Один хід гравця: двері, вихід, стіна, бій або рух
void game_move_player(Game *g, Direction d) {
    if (g->dead) return;

    Room *room = game_room(g);
    int nx = g->player.x + DIR_DX[d];
    int ny = g->player.y + DIR_DY[d];
    int wasted = 0;

    int door = room_door_dir_at(room, nx, ny);

    if (door >= 0) {
        if (room_is_open(room)) { use_door(g, (Direction)door); return; }
        wasted = 1;
        set_message(g, "The door is sealed.");
    } else if (room_exit_at(room, nx, ny)) {
        if (room_is_open(room)) { use_exit(g); return; }
        wasted = 1;
        set_message(g, "The exit is sealed.");
    } else if (nx <= 0 || ny <= 0 || nx >= ROOM_SIZE - 1 || ny >= ROOM_SIZE - 1) {
        wasted = 1;
        set_message(g, "You bump into a wall.");
    } else {
        Entity *enemy = room_enemy_at(room, nx, ny);
        if (enemy) {
            int lost = combat_player_attack(&g->player, enemy, rand() % 100);
            if (enemy->hp <= 0) {
                game_add_score(g, SCORE_KILL);

                // 50% шанс залишити зілля під убитим ворогом
                int dropped = 0;
                if (rand() % 100 < ENEMY_POTION_DROP_CHANCE)
                    dropped = room_place_item(room, ITEM_POTION, enemy->x, enemy->y, &g->player);
                set_message(g, "Enemy slain! +%d%s", SCORE_KILL,
                            dropped ? " It dropped a potion!" : "");
            } else if (lost) {
                set_message(g, "You hit the enemy, it hits back!");
            } else {
                set_message(g, "You hit the enemy cleanly!");
            }
            apply_hp_loss(g, lost);
            check_room_cleared(g);
        } else {
            g->player.x = nx;
            g->player.y = ny;
            pickup_item(g);
        }
    }

    // Хід ворогів після дії гравця
    enemies_turn(g, wasted);
}
