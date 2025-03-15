#ifndef STATE_H
#define STATE_H
#include "card.h"

typedef enum { M_HOME, M_DECKS, M_BATTLE, M_TRADE, M_EDIT_DECK } screen;

typedef struct {
  card_t card;
  uint16_t count;
} user_card;

typedef struct {
  screen current_screen;
  deck_t decks[MAX_DECKS];
  user_card user_cards[sizeof(CARDS) / sizeof(CARDS[0])];
} state_t;
#endif