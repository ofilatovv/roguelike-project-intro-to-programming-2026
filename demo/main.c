// main.c - точка входу і ігровий цикл
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "render.h"
#include "stats.h"
#include "terminal.h"

#define STATS_FILE "stats.txt"

// Очікування SPACE (1) або Q (0)
static int wait_space_or_quit(void) {
    for (;;) {
        int k = term_read_key();
        if (k == ' ') return 1;
        if (k == 'q' || k == 'Q' || k == -1) return 0;
    }
}

// Клавіша WASD у напрямок, -1 для іншої клавіші
static int key_to_direction(int k) {
    switch (k) {
    case 'w': case 'W': return DIR_UP;
    case 'd': case 'D': return DIR_RIGHT;
    case 's': case 'S': return DIR_DOWN;
    case 'a': case 'A': return DIR_LEFT;
    default:            return -1;
    }
}

int main(void) {
    srand((unsigned)time(NULL));

    Stats stats;
    stats_load(&stats, STATS_FILE);

    term_init();
    // Заставка
    render_title(&stats);
    if (!wait_space_or_quit()) return 0;

    Game game;
    game_init(&game, &stats, STATS_FILE);

    int keep_playing = 1;
    while (keep_playing) {
        game_start_run(&game);
        printf("\033[2J");

        // Один забіг
        int quit = 0;
        while (!game.dead && !quit) {
            render_game(&game);
            // Читання клавіші (гра покрокова)
            int k = term_read_key();
            if (k == 'q' || k == 'Q' || k == -1) { quit = 1; break; }
            int dir = key_to_direction(k);
            if (dir >= 0) game_move_player(&game, (Direction)dir);
        }
        if (quit) break;

        // Екран смерті
        render_game(&game);
        render_game_over(&game);
        keep_playing = wait_space_or_quit();
    }
    return 0;
}
