// stats.c - читання і запис stats.txt
#include <stdio.h>
#include "stats.h"

void stats_defaults(Stats *s) {
    s->runs_played = 0;
    s->max_score = 0;
}

// Читання файлу, при помилці нулі
int stats_load(Stats *s, const char *path) {
    stats_defaults(s);
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    int runs = 0, best = 0;

    int n = fscanf(f, " runs_played=%d max_score=%d", &runs, &best);
    fclose(f);

    if (n != 2 || runs < 0 || best < 0) return 0;
    s->runs_played = runs;
    s->max_score = best;
    return 1;
}

// Запис файлу
int stats_save(const Stats *s, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "runs_played=%d\nmax_score=%d\n", s->runs_played, s->max_score);
    fclose(f);
    return 1;
}
