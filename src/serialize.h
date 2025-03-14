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

fat_ptr<uint8_t> serialize_decks(deck_t decks[MAX_DECKS]) {
  size_t total_size = 0;
  for (size_t i = 0; i < MAX_DECKS; i++) {
    deck_t deck = decks[i];
    total_size += 1; // Card count byte
    if (!is_fat_nullptr(deck.cards)) {
      // Add name length byte + actual name bytes
      total_size += 1 + strnlen(deck.name, MAX_DECK_NAME_LENGTH);
      // Add card data
      total_size += deck.cards.size * CARD_BYTES;
    }
  }

  uint8_t *buf = new uint8_t[total_size];
  int offset = 0;
  for (size_t i = 0; i < MAX_DECKS; i++) {
    deck_t deck = decks[i];
    if (is_fat_nullptr(deck.cards)) {
      buf[offset++] = 0; // Empty deck
    } else {
      // Write card count
      uint8_t card_count = static_cast<uint8_t>(deck.cards.size);
      buf[offset++] = card_count;

      // Write name length and name
      size_t name_length = strnlen(deck.name, MAX_DECK_NAME_LENGTH);
      buf[offset++] = static_cast<uint8_t>(name_length);
      memcpy(buf + offset, deck.name, name_length);
      offset += name_length;

      // Write card data
      memcpy(buf + offset, deck.cards.ptr, deck.cards.size * CARD_BYTES);
      offset += deck.cards.size * CARD_BYTES;
    }
  }

  return {buf, total_size};
}

fat_ptr<deck_t> deserialize_decks(const fat_ptr<uint8_t> &bytes) {
  deck_t *decks = new deck_t[MAX_DECKS];
  size_t offset = 0;

  for (size_t i = 0; i < MAX_DECKS; i++) {
    uint8_t card_count = bytes.ptr[offset++];
    if (card_count == 0) {
      decks[i] = EMPTY_DECK;
    } else {
      // Read name length and name
      uint8_t name_length = bytes.ptr[offset++];
      memcpy(decks[i].name, bytes.ptr + offset, name_length);
      decks[i].name[name_length] = '\0'; // Null terminate
      offset += name_length;

      // Read card data
      card_t *cards = new card_t[card_count];
      memcpy(cards, bytes.ptr + offset, card_count * CARD_BYTES);
      decks[i].cards = fat_ptr<card_t>{cards, card_count};
      offset += card_count * CARD_BYTES;
    }
    dbg_printf("%d\n", card_count);
  }

  return {decks, MAX_DECKS};
}

#endif