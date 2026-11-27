// test_stats.c - тест файлу статистики
#include <assert.h>
#include <stdio.h>
#include "stats.h"

int main(void) {
    const char *path = "build/test_stats.tmp";
    remove(path);

    // Файлу немає
    Stats s;
    assert(stats_load(&s, path) == 0);
    assert(s.runs_played == 0 && s.max_score == 0);

    // Запис і читання
    s.runs_played = 12;
    s.max_score = 3200;
    assert(stats_save(&s, path) == 1);

    Stats r;
    assert(stats_load(&r, path) == 1);
    assert(r.runs_played == 12 && r.max_score == 3200);

    // Пошкоджений файл
    FILE *f = fopen(path, "w");
    fputs("garbage", f);
    fclose(f);
    assert(stats_load(&r, path) == 0);
    assert(r.runs_played == 0 && r.max_score == 0);

    remove(path);
    printf("test_stats: OK\n");
    return 0;
}
