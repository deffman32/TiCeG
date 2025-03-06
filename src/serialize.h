#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "card.h"
#include "utils.h"
#include <alloca.h>
#include <debug.h>
#include <stdint.h>

#define CARD_BYTES sizeof(card_t)

fat_ptr<uint8_t> serialize_cards(const fat_ptr<card_t> &cards) {
  if (!cards.ptr || cards.size == 0 || cards.size > UINT8_MAX) {
    return fat_ptr<uint8_t>{nullptr, 0};
  }

  const size_t total_size = 1 + (cards.size * CARD_BYTES);
  uint8_t *buffer = new uint8_t[total_size];

  buffer[0] = static_cast<uint8_t>(cards.size);

  memcpy(buffer + 1, cards.ptr, cards.size * CARD_BYTES);

  return fat_ptr<uint8_t>{buffer, total_size};
}

void serialize_cards(const fat_ptr<card_t> &cards, fat_ptr<uint8_t> *buf) {
  if (!cards.ptr || cards.size == 0 || cards.size > UINT8_MAX) {
    *buf = fat_ptr<uint8_t>{nullptr, 0};
    return;
  }

  buf->ptr[0] = static_cast<uint8_t>(cards.size);

  memcpy(buf->ptr + 1, cards.ptr, cards.size * CARD_BYTES);
}

fat_ptr<card_t> deserialize_cards(const fat_ptr<uint8_t> &bytes) {
  if (!bytes.ptr || bytes.size <= 1) {
    return fat_ptr<card_t>{nullptr, 0};
  }

  const uint8_t num_cards = bytes.ptr[0];
  const size_t expected_size = 1 + (num_cards * CARD_BYTES);

  if (bytes.size != expected_size) {
    return fat_ptr<card_t>{nullptr, 0};
  }

  card_t *cards = new card_t[num_cards];

  memcpy(cards, bytes.ptr + 1, num_cards * CARD_BYTES);

  return fat_ptr<card_t>{cards, num_cards};
}

const size_t MAX_DECKS = 10;

fat_ptr<uint8_t> serialize_decks(deck_t decks[10]) {
  size_t total_size = 0;
  for (int i = 0; i < 10; i++) {
    deck_t deck = decks[i];
    total_size += 1;
    if (!is_fat_nullptr(deck.cards)) {
      total_size += deck.cards.size * CARD_BYTES;
    }
  }

  uint8_t *buf = new uint8_t[total_size];
  int offset = 0;
  for (int i = 0; i < 10; i++) {
    deck_t deck = decks[i];
    if (is_fat_nullptr(deck.cards)) {
      buf[offset++] = 0;
    } else {
      size_t size = deck.cards.size * CARD_BYTES + 1;
      fat_ptr<uint8_t> ptr = {buf + offset, size};
      serialize_cards(deck.cards, &ptr);
      offset += size;
    }
  }

  return {buf, total_size};
}

fat_ptr<deck_t> deserialize_decks(const fat_ptr<uint8_t> &bytes) {
  deck_t *decks = new deck_t[10];
  size_t offset = 0;

  for (int i = 0; i < 10; i++) {
    uint8_t card_count = bytes.ptr[offset];
    if (card_count == 0) {
      decks[i] = EMPTY_DECK;
      offset += 1;
    } else {
      size_t deck_size = 1 + (card_count * CARD_BYTES);
      fat_ptr<uint8_t> card_data = {bytes.ptr + offset, deck_size};
      decks[i].cards = deserialize_cards(card_data);
      offset += deck_size;
    }
    dbg_printf("%d\n", card_count);
  }

  return {decks, 10};
}

#endif