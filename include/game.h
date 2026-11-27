// game.h - стан гри і правила ходу
#ifndef GAME_H
#define GAME_H
#include "types.h"

typedef struct {
    Entity      player;
    Floor       floor;
    int         current_room;
    int         floor_number;
    int         score;
    int         dead;
    Stats       stats;
    const char *stats_path;
    char        message[MSG_LEN];
} Game;

void game_init(Game *g, const Stats *stats, const char *stats_path);
void game_start_run(Game *g);
void game_move_player(Game *g, Direction d);
void game_add_score(Game *g, int delta);
Room *game_room(Game *g);

#endif
