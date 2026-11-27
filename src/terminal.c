// terminal.c - консоль Windows: клавіші, кольори, розмір вікна
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include "terminal.h"

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static int term_ready = 0;
static DWORD saved_out_mode;

// Відновлення консолі при виході
void term_shutdown(void) {
    if (!term_ready) return;
    term_ready = 0;
    printf("\033[0m\033[?25h\n");
    fflush(stdout);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), saved_out_mode);
}

// Ввімкнення UTF-8 і ANSI-кольорів
void term_init(void) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(CP_UTF8);
    if (!GetConsoleMode(out, &saved_out_mode)) return;
    SetConsoleMode(out, saved_out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    term_ready = 1;
    atexit(term_shutdown);
    printf("\033[?25l\033[2J\033[H");
    fflush(stdout);
}

// Читання однієї клавіші без Enter
int term_read_key(void) {
    int c = _getch();
    if (c == 0 || c == 224) { _getch(); return 0; }   // стрілки та F-клавіші ігноруються
    if (c == 3) return 'q';                           // Ctrl+C
    return c;
}

// Розмір вікна консолі
void term_size(int *w, int *h) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        *w = info.srWindow.Right - info.srWindow.Left + 1;
        *h = info.srWindow.Bottom - info.srWindow.Top + 1;
    } else {
        *w = 80;
        *h = 24;
    }
}
