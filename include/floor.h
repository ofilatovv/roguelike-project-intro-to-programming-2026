/* floor.h — випадкова генерація поверху. */
#ifndef FLOOR_H
#define FLOOR_H
#include "types.h"

/* Генерує поверх: кімнати, двері (1..3 на кімнату), вихід, ворогів. */
void floor_generate(Floor *f, int floor_number);

#endif
