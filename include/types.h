// types.h - константи і структури гри
#ifndef TYPES_H
#define TYPES_H

// Розміри та ліміти
#define ROOM_SIZE     21
#define ROOM_CENTER   (ROOM_SIZE / 2)
#define MAX_ROOMS     7
#define MIN_ROOMS     5
#define MAX_DOORS     3
#define MAX_ENEMIES   5
#define MAX_ITEMS     (MAX_ENEMIES + 1)
#define MSG_LEN       128

// Баланс
#define PLAYER_MAX_HP     5
#define ENEMY_START_HP    3
#define EXCHANGE_CHANCE   70

// Нагорода за зачистку (при HP менше 3 шанс зілля подвоюється)
#define REWARD_COIN_CHANCE        25
#define REWARD_POTION_CHANCE      25
#define LOW_HP_THRESHOLD          3
#define ENEMY_POTION_DROP_CHANCE  50

#define SCORE_KILL         100
#define SCORE_COIN         100
#define SCORE_POTION_FULL  300
#define SCORE_NEXT_FLOOR   500
#define SCORE_HP_LOSS      20

// Символи плиток
#define TILE_FLOOR   '.'
#define TILE_WALL    '#'
#define TILE_PLAYER  '@'
#define TILE_ENEMY   'E'
#define TILE_COIN    '$'
#define TILE_POTION  '+'
#define TILE_DOOR_V  '|'
#define TILE_DOOR_H  '-'
#define TILE_EXIT_R  '>'
#define TILE_EXIT_L  '<'
#define TILE_EXIT_U  '^'

typedef enum { DIR_UP = 0, DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_COUNT } Direction;

typedef enum { ITEM_NONE = 0, ITEM_COIN, ITEM_POTION } ItemType;

// Гравець або ворог
typedef struct {
    int x, y;
    int hp, max_hp;
} Entity;

// Предмет на підлозі
typedef struct {
    int x, y;
    ItemType type;
} Item;

// Клітинка екрана
typedef struct {
    char kind;
    int  open;
} Tile;

// Кімната (door_target: -1 означає, що дверей немає)
typedef struct {
    int    door_target[DIR_COUNT];
    int    exit_dir;
    Entity enemies[MAX_ENEMIES];
    int    enemy_count;
    Item   items[MAX_ITEMS];
    int    reward_rolled;
    int    grid_x, grid_y;
} Room;

// Поверх
typedef struct {
    Room rooms[MAX_ROOMS];
    int  room_count;
    int  start_room;
} Floor;

// Статистика з файлу
typedef struct {
    int runs_played;
    int max_score;
} Stats;

#endif
