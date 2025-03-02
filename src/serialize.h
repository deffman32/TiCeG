#include "card.h"
#include "utils.h"
#include <alloca.h>
#include <stdint.h>

#define CARD_STRUCT_SIZE sizeof(card_t)

fat_ptr<uint8_t> serialize_hand(const fat_ptr<card_t> &hand) {
  if (!hand.ptr || hand.size == 0 || hand.size > UINT8_MAX) {
    return fat_ptr<uint8_t>{nullptr, 0};
  }

  const size_t total_size = 1 + (hand.size * CARD_STRUCT_SIZE);
  uint8_t *buffer = new uint8_t[total_size];

  buffer[0] = static_cast<uint8_t>(hand.size);

  memcpy(buffer + 1, hand.ptr, hand.size * CARD_STRUCT_SIZE);

  return fat_ptr<uint8_t>{buffer, total_size};
}

fat_ptr<card_t> deserialize_hand(const fat_ptr<uint8_t> &bytes) {
  if (!bytes.ptr || bytes.size <= 1) {
    return fat_ptr<card_t>{nullptr, 0};
  }

  const uint8_t num_cards = bytes.ptr[0];
  const size_t expected_size = 1 + (num_cards * CARD_STRUCT_SIZE);

  if (bytes.size != expected_size) {
    return fat_ptr<card_t>{nullptr, 0};
  }

  card_t *cards = new card_t[num_cards];

  memcpy(cards, bytes.ptr + 1, num_cards * CARD_STRUCT_SIZE);

  return fat_ptr<card_t>{cards, num_cards};
}