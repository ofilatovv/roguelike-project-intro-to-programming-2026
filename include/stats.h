/* stats.h — читання та запис файлу статистики (runs played, max score). */
#ifndef STATS_H
#define STATS_H
#include "types.h"

void stats_defaults(Stats *s);                         /* нулі */
int  stats_load(Stats *s, const char *path);           /* 1 = успіх, 0 = файл відсутній/зіпсований (тоді нулі) */
int  stats_save(const Stats *s, const char *path);     /* 1 = успіх */

#endif
