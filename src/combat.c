// combat.c - формула бою
#include "combat.h"

int combat_player_attack(Entity *player, Entity *enemy, int roll) {
    enemy->hp -= 1;
    // Убитий ворог не відповідає
    if (enemy->hp <= 0) {
        enemy->hp = 0;
        return 0;
    }

    // Відповідь ворога з шансом 70%
    if (roll < EXCHANGE_CHANCE) {
        player->hp -= 1;
        if (player->hp < 0) player->hp = 0;
        return 1;
    }
    return 0;
}
