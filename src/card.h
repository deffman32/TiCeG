#ifndef CARD_H
#define CARD_H

#include "utils.h"
#include <stdint.h>

constexpr size_t MAX_DECKS = 10;

typedef struct __attribute((packed)) {
  uint8_t damage;
  uint8_t defense;
  uint16_t card_idx;
} card_t;

typedef struct {
  fat_ptr<card_t> cards;
} deck_t;

constexpr deck_t EMPTY_DECK = {fat_nullptr<card_t>};

#endif // CARD_H
