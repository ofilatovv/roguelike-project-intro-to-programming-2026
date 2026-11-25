/*
 * main.c — точка входу та ігровий цикл (game loop).
 *
 * Цикл покроковий: програма стоїть на term_read_key(), і екран
 * перемальовується лише після натискання клавіші.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "render.h"
#include "stats.h"
#include "terminal.h"

#define STATS_FILE "stats.txt"

/* Чекає пробіл (повертає 1) або Q/EOF (повертає 0). */
static int wait_space_or_quit(void) {
    for (;;) {
        int k = term_read_key();
        if (k == ' ') return 1;
        if (k == 'q' || k == 'Q' || k == -1) return 0;
    }
}

/* Переводить клавішу WASD у напрямок. Повертає -1, якщо клавіша не рух. */
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
    srand((unsigned)time(NULL));        /* різні мапи при кожному запуску */

    Stats stats;
    stats_load(&stats, STATS_FILE);     /* підтягуємо max_score і runs_played */

    term_init();
    render_title(&stats);
    if (!wait_space_or_quit()) return 0;

    Game game;
    game_init(&game, &stats, STATS_FILE);

    int keep_playing = 1;
    while (keep_playing) {
        game_start_run(&game);          /* runs_played++ і запис у файл */
        printf("\033[2J");              /* один раз повністю очищаємо екран */

        /* ---- Один забіг ---- */
        int quit = 0;
        while (!game.dead && !quit) {
            render_game(&game);
            int k = term_read_key();    /* блокуючий виклик = покрокова гра */
            if (k == 'q' || k == 'Q' || k == -1) { quit = 1; break; }
            int dir = key_to_direction(k);
            if (dir >= 0) game_move_player(&game, (Direction)dir);
        }
        if (quit) break;

        /* ---- Смерть: показуємо підсумок і питаємо, що далі ---- */
        render_game(&game);             /* останній кадр: видно причину смерті */
        render_game_over(&game);
        keep_playing = wait_space_or_quit();
    }
    return 0;
}
