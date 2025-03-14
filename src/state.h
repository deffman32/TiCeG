#include "card.h"

typedef enum { M_HOME, M_DECKS, M_BATTLE, M_TRADE, M_EDIT_DECK } screen;

typedef struct {
  screen current_screen;
  deck_t decks[MAX_DECKS];
} state_t;