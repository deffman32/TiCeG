typedef enum { M_HOME, M_DECKS, M_BATTLE, M_TRADE } screen;

typedef struct {
  screen current_screen;
} state_t;