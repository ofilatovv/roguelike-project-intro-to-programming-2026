// stats.h - файл статистики
#ifndef STATS_H
#define STATS_H
#include "types.h"

void stats_defaults(Stats *s);
int  stats_load(Stats *s, const char *path);
int  stats_save(const Stats *s, const char *path);

#endif
