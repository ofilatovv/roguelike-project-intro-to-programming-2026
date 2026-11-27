// room.h - кімната і масив 21x21
#ifndef ROOM_H
#define ROOM_H
#include "types.h"

extern const int DIR_DX[DIR_COUNT];
extern const int DIR_DY[DIR_COUNT];

Direction dir_opposite(Direction d);
void room_door_cell(Direction d, int *x, int *y);
void room_entry_cell(Direction d, int *x, int *y);

void room_init(Room *r);
void room_spawn_enemies(Room *r, int floor_number);
int  room_alive_enemies(const Room *r);
int  room_is_open(const Room *r);
Entity *room_enemy_at(Room *r, int x, int y);
int  room_door_dir_at(const Room *r, int x, int y);
int  room_exit_at(const Room *r, int x, int y);
Item *room_item_at(Room *r, int x, int y);

int  room_place_item(Room *r, ItemType type, int cx, int cy, const Entity *player);

ItemType room_roll_reward(Room *r, const Entity *player);

void room_build_grid(const Room *r, const Entity *player, Tile grid[ROOM_SIZE][ROOM_SIZE]);

#endif
