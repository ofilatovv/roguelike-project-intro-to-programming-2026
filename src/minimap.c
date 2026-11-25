/*
 * minimap.c — перетворює позиції кімнат (grid_x, grid_y) на невелику схему.
 *
 * Кімната = клітинка з ПАРНИМИ координатами (0,2,4..), а між двома сусідніми
 * кімнатами лежить клітинка-проміжок з непарною координатою: у ній малюється
 * лінія, якщо між кімнатами є двері. Так видно, які кімнати з'єднані
 * насправді, а які просто стоять поруч.
 */
#include "minimap.h"

/* Межі займаної області на сітці поверху. */
static void bounds(const Floor *f, int *minx, int *miny, int *maxx, int *maxy) {
    *minx = *maxx = f->rooms[0].grid_x;
    *miny = *maxy = f->rooms[0].grid_y;
    for (int i = 1; i < f->room_count; i++) {
        const Room *r = &f->rooms[i];
        if (r->grid_x < *minx) *minx = r->grid_x;
        if (r->grid_x > *maxx) *maxx = r->grid_x;
        if (r->grid_y < *miny) *miny = r->grid_y;
        if (r->grid_y > *maxy) *maxy = r->grid_y;
    }
}

void minimap_cell_of(const Floor *f, int room, int *cx, int *cy) {
    int minx, miny, maxx, maxy;
    bounds(f, &minx, &miny, &maxx, &maxy);
    *cx = (f->rooms[room].grid_x - minx) * 2;
    *cy = (f->rooms[room].grid_y - miny) * 2;
}

/* Кімната "зачищена", коли в ній не лишилось живих ворогів
 * (стартова кімната без ворогів зачищена з самого початку). */
static int is_cleared(const Room *r) {
    for (int i = 0; i < r->enemy_count; i++)
        if (r->enemies[i].hp > 0) return 0;
    return 1;
}

void minimap_build(const Floor *f, int current_room,
                   MiniCell out[MINIMAP_MAX][MINIMAP_MAX], int *w, int *h) {
    for (int y = 0; y < MINIMAP_MAX; y++)
        for (int x = 0; x < MINIMAP_MAX; x++) {
            out[y][x].kind = MINI_NONE;
            out[y][x].current = 0;
        }

    int minx, miny, maxx, maxy;
    bounds(f, &minx, &miny, &maxx, &maxy);
    *w = (maxx - minx) * 2 + 1;
    *h = (maxy - miny) * 2 + 1;

    for (int i = 0; i < f->room_count; i++) {
        const Room *r = &f->rooms[i];
        int cx, cy;
        minimap_cell_of(f, i, &cx, &cy);

        out[cy][cx].kind = is_cleared(r) ? MINI_ROOM_FILLED : MINI_ROOM_EMPTY;
        out[cy][cx].current = (i == current_room);

        /* Прохід малюємо лише вправо та вниз, щоб кожну лінію поставити один раз. */
        if (r->door_target[DIR_RIGHT] >= 0) out[cy][cx + 1].kind = MINI_LINK_H;
        if (r->door_target[DIR_DOWN]  >= 0) out[cy + 1][cx].kind = MINI_LINK_V;
    }
}
