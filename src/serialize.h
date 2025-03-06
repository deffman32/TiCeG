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

void serialize_cards(const fat_ptr<card_t> &cards, fat_ptr<uint8_t> *buf) {
  if (!cards.ptr || cards.size == 0 || cards.size > UINT8_MAX ||
      ((1 + (cards.size * sizeof(card_t))) != buf->size)) {
    *buf = fat_ptr<uint8_t>{nullptr, 0};
    return;
  }

  buf->ptr[0] = static_cast<uint8_t>(cards.size);

  memcpy(buf->ptr + 1, cards.ptr, cards.size * sizeof(card_t));
}

fat_ptr<card_t> deserialize_cards(const fat_ptr<uint8_t> &bytes) {
  if (!bytes.ptr || bytes.size <= 2) {
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

const size_t MAX_DECKS = 10;

fat_ptr<uint8_t> serialize_decks(deck_t decks[10]) {
  size_t total_size = 0;
  for (int i = 0; i < 10; i++) {
    deck_t deck = decks[i];
    if (is_fat_nullptr(deck.cards)) {
      total_size += 1;
    }
  }

  uint8_t *buf = new uint8_t[total_size];
  int offset = 0;
  for (int i = 0; i < 10; i++) {
    deck_t deck = decks[i];
    if (is_fat_nullptr(deck.cards)) {
      buf[offset++] = 0;
    } else {
      fat_ptr<uint8_t> ptr = {buf + offset, deck.cards.size * sizeof(card_t)};
      serialize_cards(deck.cards, &ptr);
    }
  }

  return {buf, total_size};
}

fat_ptr<deck_t> deserialize_decks(const fat_ptr<uint8_t> &bytes) {
  size_t total_size = 0;
  for (int i = 0; i < 10; i++) {
    size_t size = bytes.ptr[total_size] * sizeof(card_t) + 1;
    total_size += size;
  }

  deck_t *decks = new deck_t[10];
  int offset = 0;
  for (int i = 0; i < 10; i++) {
    size_t size = bytes.ptr[offset] * sizeof(card_t) + 1;
    if (size == 1) {
      decks[i] = EMPTY_DECK;
    } else {
      fat_ptr<uint8_t> ptr = {bytes.ptr + offset, size};
      decks[i] = {deserialize_cards(ptr)};
    }
    offset += size;
  }

  return {decks, 10};
}