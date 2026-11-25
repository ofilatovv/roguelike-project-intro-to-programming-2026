/* game.h — стан гри та ігрова логіка (покроковий хід). */
#ifndef GAME_H
#define GAME_H
#include "types.h"

typedef struct {
    Entity      player;
    Floor       floor;
    int         current_room;
    int         floor_number;
    int         score;            /* current_score — живе лише один забіг */
    int         dead;
    Stats       stats;            /* копія статистики з файлу */
    const char *stats_path;       /* куди зберігати (NULL = не зберігати, для тестів) */
    char        message[MSG_LEN]; /* короткий рядок-підказка про останню подію */
} Game;

void game_init(Game *g, const Stats *stats, const char *stats_path);
void game_start_run(Game *g);                 /* новий забіг: runs_played++, очки = 0, 1-й поверх */
void game_move_player(Game *g, Direction d);  /* один повний хід: дія гравця + хід ворогів */
void game_add_score(Game *g, int delta);      /* змінює очки та оновлює max_score */
Room *game_room(Game *g);                     /* поточна кімната */

#endif
