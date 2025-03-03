#include "card.h"
#include "utils.h"
#include <alloca.h>
#include <stdint.h>

fat_ptr<uint8_t> serialize_cards(const fat_ptr<card_t> &cards) {
  if (!cards.ptr || cards.size == 0 || cards.size > UINT8_MAX) {
    return fat_ptr<uint8_t>{nullptr, 0};
  }

  const size_t total_size = 1 + (cards.size * sizeof(card_t));
  uint8_t *buffer = new uint8_t[total_size];

  buffer[0] = static_cast<uint8_t>(cards.size);

  memcpy(buffer + 1, cards.ptr, cards.size * sizeof(card_t));

  return fat_ptr<uint8_t>{buffer, total_size};
}

fat_ptr<card_t> deserialize_cards(const fat_ptr<uint8_t> &bytes) {
  if (!bytes.ptr || bytes.size <= 1) {
    return fat_ptr<card_t>{nullptr, 0};
  }

  const uint8_t num_cards = bytes.ptr[0];
  const size_t expected_size = 1 + (num_cards * sizeof(card_t));

  if (bytes.size != expected_size) {
    return fat_ptr<card_t>{nullptr, 0};
  }

  card_t *cards = new card_t[num_cards];

  memcpy(cards, bytes.ptr + 1, num_cards * sizeof(card_t));

  return fat_ptr<card_t>{cards, num_cards};
}