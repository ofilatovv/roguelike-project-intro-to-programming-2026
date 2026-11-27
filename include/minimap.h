// minimap.h - дані мінімапи
#ifndef MINIMAP_H
#define MINIMAP_H
#include "types.h"

#define MINIMAP_MAX (2 * MAX_ROOMS - 1)

typedef enum {
    MINI_NONE = 0,
    MINI_ROOM_EMPTY,
    MINI_ROOM_FILLED,
    MINI_LINK_H,
    MINI_LINK_V
} MiniKind;

typedef struct {
    MiniKind kind;
    int      current;
} MiniCell;

void minimap_build(const Floor *f, int current_room,
                   MiniCell out[MINIMAP_MAX][MINIMAP_MAX], int *w, int *h);

void minimap_cell_of(const Floor *f, int room, int *cx, int *cy);

#endif
