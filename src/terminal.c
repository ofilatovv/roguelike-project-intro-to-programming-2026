/*
 * terminal.c — низькорівневе керування терміналом (Windows + Linux / macOS / WSL).
 * Гра покрокова, тому читаємо клавіші блокуюче: екран оновлюється лише після натискання.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "terminal.h"

#ifdef _WIN32
/* ======================= Windows-версія (консоль cmd / PowerShell) ======================= */
#include <conio.h>
#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004   /* для старих заголовків MinGW */
#endif

static int  term_ready = 0;
static DWORD saved_out_mode;

void term_shutdown(void) {
    if (!term_ready) return;
    term_ready = 0;
    printf("\033[0m\033[?25h\n");                       /* скинути колір, показати курсор */
    fflush(stdout);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), saved_out_mode);
}

void term_init(void) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(CP_UTF8);                        /* щоб довге тире "—" малювалось коректно */
    if (!GetConsoleMode(out, &saved_out_mode)) return;
    /* Вмикаємо ANSI-кольори та керування курсором у консолі Windows 10+ */
    SetConsoleMode(out, saved_out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    term_ready = 1;
    atexit(term_shutdown);
    printf("\033[?25l\033[2J\033[H");                   /* сховати курсор, очистити екран */
    fflush(stdout);
}

int term_read_key(void) {
    int c = _getch();                                   /* читає клавішу без ехо та без Enter */
    if (c == 0 || c == 224) { _getch(); return 0; }     /* стрілки/F-клавіші: ігноруємо */
    if (c == 3) return 'q';                             /* Ctrl+C -> вихід */
    return c;
}

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

#else
/* ======================= POSIX-версія (Linux / macOS / WSL) ======================= */
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static struct termios saved_termios;
static int raw_enabled = 0;

/* Повертає термінал у нормальний стан (ехо, буферизація, курсор). */
void term_shutdown(void) {
    if (!raw_enabled) return;
    raw_enabled = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_termios);
    const char *seq = "\033[0m\033[?25h\n";   /* скинути колір, показати курсор */
    if (write(STDOUT_FILENO, seq, strlen(seq)) < 0) { /* ігноруємо помилку запису */ }
}

/* Ctrl+C: акуратно відновлюємо термінал перед виходом. */
static void on_sigint(int sig) {
    (void)sig;
    term_shutdown();
    _exit(0);
}

void term_init(void) {
    if (tcgetattr(STDIN_FILENO, &saved_termios) != 0) return;   /* stdin не термінал */

    struct termios raw = saved_termios;
    raw.c_lflag &= ~(ICANON | ECHO);   /* без рядкового буфера та без ехо */
    raw.c_cc[VMIN] = 1;                /* read() чекає рівно 1 символ */
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_enabled = 1;

    atexit(term_shutdown);
    signal(SIGINT, on_sigint);

    const char *seq = "\033[?25l\033[2J\033[H";   /* сховати курсор, очистити екран */
    if (write(STDOUT_FILENO, seq, strlen(seq)) < 0) { }
}

int term_read_key(void) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) <= 0) return -1;

    /* Стрілки надсилають ESC [ A.. — "з'їдаємо" хвіст, щоб 'A' не стало ходом вліво. */
    if (c == 27) {
        struct pollfd p = { STDIN_FILENO, POLLIN, 0 };
        while (poll(&p, 1, 10) > 0) {
            unsigned char junk;
            if (read(STDIN_FILENO, &junk, 1) <= 0) break;
        }
        return 0;
    }
    return c;
}

void term_size(int *w, int *h) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        *w = ws.ws_col;
        *h = ws.ws_row;
    } else {
        *w = 80;    /* запасні значення, якщо розмір дізнатись не вдалось */
        *h = 24;
    }
}

#endif /* _WIN32 */
