/*
 * combat.c — правила бою.
 * Логіка винесена окремо від вводу/виводу, щоб її легко тестувати assert-ами.
 */
#include "combat.h"

int combat_player_attack(Entity *player, Entity *enemy, int roll) {
    /* Ворог отримує 1 одиницю урону завжди. */
    enemy->hp -= 1;
    if (enemy->hp <= 0) {
        enemy->hp = 0;
        return 0;               /* убитий ворог не може вдарити у відповідь */
    }

    /* 70%: ворог б'є у відповідь; 30%: гравець отримує безкарний удар. */
    if (roll < EXCHANGE_CHANCE) {
        player->hp -= 1;
        if (player->hp < 0) player->hp = 0;
        return 1;
    }
    return 0;
}
