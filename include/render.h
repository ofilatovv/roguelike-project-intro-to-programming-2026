/* render.h — малювання екранів у терміналі (ANSI-кольори). */
#ifndef RENDER_H
#define RENDER_H
#include "types.h"
#include "game.h"

void render_title(const Stats *stats);
void render_game(Game *g);
void render_game_over(const Game *g);

#endif
