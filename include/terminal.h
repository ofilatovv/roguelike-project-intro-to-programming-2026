// terminal.h - консоль Windows
#ifndef TERMINAL_H
#define TERMINAL_H

void term_init(void);
void term_shutdown(void);
int  term_read_key(void);
void term_size(int *w, int *h);

#endif
