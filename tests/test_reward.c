// test_reward.c - тест нагород
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "room.h"

static void roll_many(int hp, int n, int *potions, int *coins) {
    Entity p = { 1, 1, hp, PLAYER_MAX_HP };
    *potions = *coins = 0;
    for (int i = 0; i < n; i++) {
        Room r;
        room_init(&r);
        ItemType t = room_roll_reward(&r, &p);
        if (t == ITEM_POTION) (*potions)++;
        if (t == ITEM_COIN)   (*coins)++;
    }
}

int main(void) {
    srand(12345);
    const int N = 8000;
    int potions, coins;

    // HP 3 і більше: шанс зілля 25%
    int hps_normal[] = { 5, 4, 3 };
    for (int i = 0; i < 3; i++) {
        roll_many(hps_normal[i], N, &potions, &coins);
        assert(potions > N * 22 / 100 && potions < N * 28 / 100);
        assert(coins   > N * 22 / 100 && coins   < N * 28 / 100);
    }

    // HP менше 3: шанс зілля 50%
    int hps_low[] = { 2, 1 };
    for (int i = 0; i < 2; i++) {
        roll_many(hps_low[i], N, &potions, &coins);
        assert(potions > N * 47 / 100 && potions < N * 53 / 100);
        assert(coins   > N * 22 / 100 && coins   < N * 28 / 100);
    }

    // Нагорода не лягає на клітинку гравця
    Room r;
    Entity p = { ROOM_CENTER, ROOM_CENTER, 2, PLAYER_MAX_HP };
    room_init(&r);
    srand(1);
    for (int i = 0; i < 50 && room_item_at(&r, 0, 0) == NULL; i++) {
        room_init(&r);
        if (room_roll_reward(&r, &p) != ITEM_NONE) {
            for (int k = 0; k < MAX_ITEMS; k++)
                if (r.items[k].type != ITEM_NONE)
                    assert(!(r.items[k].x == p.x && r.items[k].y == p.y));
        }
    }

    printf("test_reward: OK\n");
    return 0;
}
