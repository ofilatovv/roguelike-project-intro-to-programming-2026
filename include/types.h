/*
 * types.h — спільні константи та базові структури гри.
 * Усі інші модулі підключають цей файл, щоб мати єдине "джерело правди"
 * про розміри кімнати, початкове HP, нагороди за очки тощо.
 */
#ifndef TYPES_H
#define TYPES_H

/* ---------- Розміри та ліміти ---------- */
#define ROOM_SIZE     21               /* кімната 21x21 (непарне -> двері рівно по центру) */
#define ROOM_CENTER   (ROOM_SIZE / 2)  /* індекс центральної клітинки = 10 */
#define MAX_ROOMS     7                /* максимум кімнат на поверсі */
#define MIN_ROOMS     5                /* мінімум кімнат, який ми намагаємось згенерувати */
#define MAX_DOORS     3                /* максимум дверей в одній кімнаті */
#define MAX_ENEMIES   5                /* максимум ворогів в одній кімнаті */
#define MAX_ITEMS     (MAX_ENEMIES + 1) /* дропи з усіх ворогів + нагорода за зачистку */
#define MSG_LEN       128              /* довжина рядка зі службовим повідомленням */

/* ---------- Параметри балансу ---------- */
#define PLAYER_MAX_HP     5
#define ENEMY_START_HP    3
#define EXCHANGE_CHANCE   70   /* % шансу, що ворог дасть здачі при ударі гравця */

/* Нагорода після зачистки кімнати (кидається один раз за кімнату):
 *   0..24 -> монета, 25..49 -> зілля, решта -> нічого.
 * Якщо HP гравця СТРОГО менше LOW_HP_THRESHOLD, шанс зілля подвоюється (25% -> 50%). */
#define REWARD_COIN_CHANCE        25
#define REWARD_POTION_CHANCE      25
#define LOW_HP_THRESHOLD          3
#define ENEMY_POTION_DROP_CHANCE  50   /* % шансу, що вбитий ворог залишить зілля */

#define SCORE_KILL         100
#define SCORE_COIN         100
#define SCORE_POTION_FULL  300 /* зілля при повному HP дає очки замість лікування */
#define SCORE_NEXT_FLOOR   500
#define SCORE_HP_LOSS      20  /* штраф за кожну втрачену одиницю HP */

/* ---------- Символи клітинок (логічні типи плиток) ---------- */
#define TILE_FLOOR   '.'
#define TILE_WALL    '#'
#define TILE_PLAYER  '@'
#define TILE_ENEMY   'E'
#define TILE_COIN    '$'
#define TILE_POTION  '+'
#define TILE_DOOR_V  '|'   /* двері на лівій/правій стіні */
#define TILE_DOOR_H  '-'   /* двері на верхній/нижній стіні (малюється як "—") */
#define TILE_EXIT_R  '>'
#define TILE_EXIT_L  '<'
#define TILE_EXIT_U  '^'

/* ---------- Напрямки ---------- */
typedef enum { DIR_UP = 0, DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_COUNT } Direction;

/* ---------- Типи предметів-нагород ---------- */
typedef enum { ITEM_NONE = 0, ITEM_COIN, ITEM_POTION } ItemType;

/* Сутність з HP і координатами: і гравець "@", і ворог "E". */
typedef struct {
    int x, y;
    int hp, max_hp;
} Entity;

/* Предмет на підлозі: монета або зілля (мають координати). */
typedef struct {
    int x, y;
    ItemType type;
} Item;

/* Одна клітинка двовимірного масиву-екрану. */
typedef struct {
    char kind;  /* яка структура стоїть у клітинці (один із TILE_*) */
    int  open;  /* для дверей/виходу: 1 = відкрито, 0 = зачинено */
} Tile;

/* Кімната: двері, вороги, нагорода. */
typedef struct {
    int    door_target[DIR_COUNT]; /* індекс кімнати за дверима або -1, якщо дверей немає */
    int    exit_dir;               /* стіна з виходом на наступний поверх (-1 = немає) */
    Entity enemies[MAX_ENEMIES];
    int    enemy_count;            /* скільки ворогів було створено (живих чи мертвих) */
    Item   items[MAX_ITEMS];       /* предмети на підлозі (type == ITEM_NONE — вільний слот) */
    int    reward_rolled;          /* чи вже кидали кубик на нагороду */
    int    grid_x, grid_y;         /* позиція кімнати на "карті поверху" (для генерації) */
} Room;

/* Поверх = набір кімнат (до 7). */
typedef struct {
    Room rooms[MAX_ROOMS];
    int  room_count;
    int  start_room;
} Floor;

/* Статистика, що зберігається у файлі між запусками. */
typedef struct {
    int runs_played;
    int max_score;
} Stats;

#endif
