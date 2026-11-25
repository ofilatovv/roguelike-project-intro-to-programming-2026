/*
 * stats.c — збереження статистики у текстовому файлі:
 *     runs_played=12
 *     max_score=3200
 */
#include <stdio.h>
#include "stats.h"

/* Скидає статистику до нулів (використовується, якщо файлу ще немає). */
void stats_defaults(Stats *s) {
    s->runs_played = 0;
    s->max_score = 0;
}

/* Читає файл. Якщо його немає або він пошкоджений — повертає нулі. */
int stats_load(Stats *s, const char *path) {
    stats_defaults(s);
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    int runs = 0, best = 0;
    /* пробіли у форматі пропускають будь-які розділювачі рядків */
    int n = fscanf(f, " runs_played=%d max_score=%d", &runs, &best);
    fclose(f);

    if (n != 2 || runs < 0 || best < 0) return 0;
    s->runs_played = runs;
    s->max_score = best;
    return 1;
}

/* Перезаписує файл поточними значеннями. */
int stats_save(const Stats *s, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "runs_played=%d\nmax_score=%d\n", s->runs_played, s->max_score);
    fclose(f);
    return 1;
}
