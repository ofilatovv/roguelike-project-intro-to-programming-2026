/* room.h — операції над однією кімнатою та побудова масиву-екрану 21x21. */
#ifndef ROOM_H
#define ROOM_H
#include "types.h"

extern const int DIR_DX[DIR_COUNT];
extern const int DIR_DY[DIR_COUNT];

Direction dir_opposite(Direction d);
void room_door_cell(Direction d, int *x, int *y);   /* клітинка дверей у стіні */
void room_entry_cell(Direction d, int *x, int *y);  /* клітинка одразу всередині кімнати біля дверей */

void room_init(Room *r);                            /* порожня кімната без дверей/ворогів */
void room_spawn_enemies(Room *r, int floor_number); /* створює 1..N ворогів у випадкових місцях */
int  room_alive_enemies(const Room *r);
int  room_is_open(const Room *r);                   /* двері/вихід відкриті, лише якщо ворогів немає */
Entity *room_enemy_at(Room *r, int x, int y);       /* живий ворог у клітинці або NULL */
int  room_door_dir_at(const Room *r, int x, int y); /* напрямок дверей у цій клітинці або -1 */
int  room_exit_at(const Room *r, int x, int y);     /* 1, якщо у клітинці вихід на наступний поверх */
Item *room_item_at(Room *r, int x, int y);          /* предмет у клітинці або NULL */
/* Кладе предмет у (cx,cy) або в найближчу вільну клітинку. 1 = успіх, 0 = немає місця. */
int  room_place_item(Room *r, ItemType type, int cx, int cy, const Entity *player);
/* Нагорода за зачистку: монета 25%, зілля 25% (50%, якщо HP гравця < 3), інакше нічого. */
ItemType room_roll_reward(Room *r, const Entity *player);

/* Заповнює двовимірний масив-екран за поточним станом кімнати. */
void room_build_grid(const Room *r, const Entity *player, Tile grid[ROOM_SIZE][ROOM_SIZE]);

#endif
