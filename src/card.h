#ifndef CARD_H
#define CARD_H

#include <stdint.h>

typedef struct __attribute((packed)) {
  uint8_t damage;
  uint8_t defense;
  uint16_t card_idx;
} card_t;

#endif // CARD_H
