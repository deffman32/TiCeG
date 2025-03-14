#ifndef CARD_H
#define CARD_H

#include "utils.h"
#include <stdint.h>

#define MAX_DECKS 10
#define MAX_DECK_NAME_LENGTH 32

typedef struct __attribute((packed)) {
  uint8_t damage;
  uint8_t defense;
  uint16_t card_idx;
} card_t;

typedef struct {
  fat_ptr<card_t> cards;
  char name[MAX_DECK_NAME_LENGTH];
} deck_t;

constexpr deck_t EMPTY_DECK = {fat_nullptr<card_t>, {}};

#endif // CARD_H
