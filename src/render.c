// render.c - малювання екранів
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "render.h"
#include "minimap.h"
#include "room.h"
#include "terminal.h"

// ANSI-кольори
#define C_RESET   "\033[0m"
#define C_GRAY    "\033[90m"
#define C_WHITE   "\033[97m"
#define C_RED     "\033[91m"
#define C_CYAN    "\033[96m"
#define C_YELLOW  "\033[93m"
#define C_GREEN   "\033[92m"
#define C_MAGENTA "\033[95m"

// Налаштування мінімапи
#define MINIMAP_EMPTY_GLYPH   "◻"
#define MINIMAP_FILLED_GLYPH  "■"
#define MINIMAP_SHOW_LINKS    1
#define MINIMAP_PANEL_COLS    (MINIMAP_MAX * 2 + 2)
#define MIN_TERM_WIDTH        100
#define MIN_TERM_HEIGHT       24

// Буфер кадру, вивід одним викликом
static char frame[16384];
static size_t frame_len;

static void put(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(frame + frame_len, sizeof frame - frame_len, fmt, ap);
    va_end(ap);
    if (n > 0) {
        frame_len += (size_t)n;
        if (frame_len >= sizeof frame) frame_len = sizeof frame - 1;
    }
}

static void flush_frame(void) {
    fwrite(frame, 1, frame_len, stdout);
    fflush(stdout);
    frame_len = 0;
}

// Символ і колір плитки
static void tile_style(const Tile *t, const char **glyph, const char **color) {
    switch (t->kind) {
    case TILE_WALL:   *glyph = "#";  *color = C_GRAY;    break;
    case TILE_FLOOR:  *glyph = "."; *color = C_WHITE;   break;
    case TILE_PLAYER: *glyph = "@";  *color = C_CYAN;    break;
    case TILE_ENEMY:  *glyph = "E";  *color = C_RED;     break;
    case TILE_COIN:   *glyph = "$";  *color = C_YELLOW;  break;
    case TILE_POTION: *glyph = "+";  *color = C_GREEN;   break;
    case TILE_DOOR_V: *glyph = "|";  *color = t->open ? C_WHITE : C_GRAY; break;
    case TILE_DOOR_H: *glyph = "—";  *color = t->open ? C_WHITE : C_GRAY; break;
    case TILE_EXIT_R: *glyph = ">";  *color = C_MAGENTA; break;
    case TILE_EXIT_L: *glyph = "<";  *color = C_MAGENTA; break;
    case TILE_EXIT_U: *glyph = "^";  *color = C_MAGENTA; break;
    default:          *glyph = "?";  *color = C_WHITE;   break;
    }
}

// Один рядок мінімапи
static void put_minimap_row(MiniCell cells[MINIMAP_MAX][MINIMAP_MAX], int w, int h, int y) {
    int top = (ROOM_SIZE - h) / 2;
    int my = y - top;
    int left = (MINIMAP_MAX - w) / 2;
    int used = 0;

    if (my >= 0 && my < h) {
        put("%*s", left * 2, "");
        used = left * 2;
        for (int x = 0; x < w; x++) {
            const MiniCell *c = &cells[my][x];
            switch (c->kind) {
            case MINI_ROOM_EMPTY:
            case MINI_ROOM_FILLED: {
                const char *glyph = (c->kind == MINI_ROOM_FILLED) ? MINIMAP_FILLED_GLYPH
                                                                  : MINIMAP_EMPTY_GLYPH;
                const char *color = c->current ? C_CYAN
                                  : (c->kind == MINI_ROOM_FILLED) ? C_WHITE : C_GRAY;
                put("%s%s " C_RESET, color, glyph);
                break;
            }
            case MINI_LINK_H: put(MINIMAP_SHOW_LINKS ? C_GRAY "──" C_RESET : "  "); break;
            case MINI_LINK_V: put(MINIMAP_SHOW_LINKS ? C_GRAY "│ " C_RESET : "  "); break;
            default:          put("  "); break;
            }
            used += 2;
        }
    }
    put("%*s", MINIMAP_PANEL_COLS - used, "");
}

// Заставка
void render_title(const Stats *stats) {
    int w, h;
    term_size(&w, &h);
    const char *title = "Roguelike project";
    int col = (w - (int)strlen(title)) / 2 + 1;
    if (col < 1) col = 1;

    put("\033[2J");
    put("\033[%d;%dH" C_WHITE "%s" C_RESET, h / 2, col, title);
    put("\033[%d;1HMax score: %d",     h - 3, stats->max_score);
    put("\033[%d;1HRuns played: %d",   h - 2, stats->runs_played);
    put("\033[%d;1HPress SPACE to start", h - 1);
    flush_frame();
}

// Ігровий екран: мінімапа, кімната, HUD
void render_game(Game *g) {
    Tile grid[ROOM_SIZE][ROOM_SIZE];
    room_build_grid(game_room(g), &g->player, grid);

    MiniCell mini[MINIMAP_MAX][MINIMAP_MAX];
    int mw, mh;
    minimap_build(&g->floor, g->current_room, mini, &mw, &mh);

    put("\033[H");

    for (int y = 0; y < ROOM_SIZE; y++) {
        // Мінімапа зліва
        put_minimap_row(mini, mw, mh, y);

        // Кімната посередині
        for (int x = 0; x < ROOM_SIZE; x++) {
            const char *glyph, *color;
            tile_style(&grid[y][x], &glyph, &color);
            put("%s%s " C_RESET, color, glyph);
        }

        // HUD справа
        put("   ");
        switch (y) {
        case 2:  put("SCORE: %d", g->score); break;
        case 4:  put("HP: %d/%d", g->player.hp, g->player.max_hp); break;
        case 6:  put("FLOOR: %d", g->floor_number); break;
        case 18: put(C_GRAY "WASD - move, Q - quit" C_RESET); break;
        case 19: put(C_GRAY "Walk into E to attack" C_RESET); break;
        default: break;
        }
        put("\033[K\n");
    }

    put("%s\033[K\n", g->message);

    int tw, th;
    term_size(&tw, &th);
    if (tw < MIN_TERM_WIDTH || th < MIN_TERM_HEIGHT)
        put(C_RED "Terminal too small: need at least %dx%d (now %dx%d)" C_RESET "\033[K\n",
            MIN_TERM_WIDTH, MIN_TERM_HEIGHT, tw, th);

    put("\033[J");
    flush_frame();
}

// Екран смерті
void render_game_over(const Game *g) {
    int w, h;
    term_size(&w, &h);
    (void)w;

    put("\033[2J");
    put("\033[%d;1H" C_RED "YOU DIED" C_RESET, h / 2 - 2);
    put("\033[%d;1HFloor reached: %d", h / 2, g->floor_number);
    put("\033[%d;1HScore: %d",         h / 2 + 1, g->score);
    put("\033[%d;1HMax score: %d",     h / 2 + 2, g->stats.max_score);
    put("\033[%d;1HPress SPACE to play again or Q to quit", h / 2 + 4);
    flush_frame();
}
