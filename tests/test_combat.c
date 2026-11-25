/* test_combat.c — перевірка правил бою (70% обмін / 30% безкарний удар). */
#include <assert.h>
#include <stdio.h>
#include "combat.h"

int main(void) {
    Entity p = { 1, 1, 5, 5 };
    Entity e = { 2, 1, 3, 3 };

    /* roll < 70: обмін ударами — обидва втрачають по 1 HP */
    assert(combat_player_attack(&p, &e, 0) == 1);
    assert(p.hp == 4 && e.hp == 2);

    /* межа: 69 ще обмін, 70 уже безкарний удар */
    assert(combat_player_attack(&p, &e, 69) == 1);
    assert(p.hp == 3 && e.hp == 1);

    p.hp = 5; e.hp = 3;
    assert(combat_player_attack(&p, &e, 70) == 0);
    assert(p.hp == 5 && e.hp == 2);

    assert(combat_player_attack(&p, &e, 99) == 0);
    assert(p.hp == 5 && e.hp == 1);

    /* убитий ворог не б'є у відповідь, навіть якщо roll "поганий" */
    assert(combat_player_attack(&p, &e, 0) == 0);
    assert(e.hp == 0 && p.hp == 5);

    printf("test_combat: OK\n");
    return 0;
}
