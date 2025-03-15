#ifndef CARD_H
#define CARD_H

#include "utils.h"
#include <graphx.h>
#include <stdint.h>

#define MAX_DECKS 10
#define MAX_DECK_NAME_LENGTH 32

typedef uint16_t card_t;

typedef struct {
  fat_ptr<card_t> cards;
  char name[MAX_DECK_NAME_LENGTH];
} deck_t;

constexpr deck_t EMPTY_DECK = {fat_nullptr<card_t>, {}};

typedef struct {
  uint8_t damage;
  uint8_t defense;
  const char *name;
  gfx_sprite_t *sprite;
} card_tuple_t;

card_tuple_t CARDS[] = {{10, 3, "Test Card 1", nullptr},
                        {5, 6, "Test Card 2", nullptr},
                        {7, 2, "Test Card 3", nullptr},
                        {1, 4, "Test Card 1", nullptr}};

#endif // CARD_H
