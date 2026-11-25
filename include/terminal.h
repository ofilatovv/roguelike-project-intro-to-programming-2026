/* terminal.h — робота з терміналом (POSIX): сирий режим, читання клавіш, розмір. */
#ifndef TERMINAL_H
#define TERMINAL_H

void term_init(void);            /* вимикає ехо/буферизацію, ховає курсор; відновлення — автоматично при виході */
void term_shutdown(void);
int  term_read_key(void);        /* блокуюче читання 1 клавіші; -1 = EOF; 0 = службова (стрілки тощо) */
void term_size(int *w, int *h);  /* розмір вікна в символах */

#endif
