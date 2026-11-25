/* minimap.h — схема кімнат поверху (дані для мінімапи, без виводу в термінал). */
#ifndef MINIMAP_H
#define MINIMAP_H
#include "types.h"

/* Максимальний розмір: 7 кімнат у ряд + 6 проміжків між ними = 13 клітинок. */
#define MINIMAP_MAX (2 * MAX_ROOMS - 1)

typedef enum {
    MINI_NONE = 0,       /* порожнє місце */
    MINI_ROOM_EMPTY,     /* кімната ще не зачищена  (малюється ◻, сірий) */
    MINI_ROOM_FILLED,    /* кімната зачищена        (малюється ■, білий) */
    MINI_LINK_H,         /* прохід між кімнатами по горизонталі */
    MINI_LINK_V          /* прохід між кімнатами по вертикалі */
} MiniKind;

typedef struct {
    MiniKind kind;
    int      current;    /* 1 = у цій кімнаті зараз гравець (підсвічується кольором гравця) */
} MiniCell;

/* Будує схему; w, h — реальні розміри схеми в клітинках (кімнати стоять через одну клітинку). */
void minimap_build(const Floor *f, int current_room,
                   MiniCell out[MINIMAP_MAX][MINIMAP_MAX], int *w, int *h);

/* У яку клітинку схеми потрапила кімната room (потрібно для тестів). */
void minimap_cell_of(const Floor *f, int room, int *cx, int *cy);

#endif
