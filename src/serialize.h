#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "card.h"
#include "state.h"
#include "utils.h"
#include <alloca.h>
#include <ctype.h>
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
  // First pass: calculate total size needed
  size_t total_size = 0;

  // Start with a header containing the number of decks
  total_size = sizeof(uint8_t);

  // Structure: For each deck [card_count(1) + name_length(1) + name(var) +
  // cards(var)]
  for (size_t i = 0; i < MAX_DECKS; i++) {
    // Always add 1 byte for card count
    total_size += sizeof(uint8_t);

    // Calculate name length
    size_t name_length = strnlen(decks[i].name, MAX_DECK_NAME_LENGTH);
    bool is_blank = (name_length == 0);

    if (!is_blank) {
      // Check if name is only whitespace
      bool only_whitespace = true;
      for (size_t j = 0; j < name_length; j++) {
        if (!isspace(decks[i].name[j])) {
          only_whitespace = false;
          break;
        }
      }
      is_blank = only_whitespace;
    }

    // Default name if blank
    char default_name[16];
    if (is_blank) {
      sprintf(default_name, "Deck %u", i + 1);
      name_length = strlen(default_name);
    }

    // Add 1 byte for name length + actual name bytes
    total_size += sizeof(uint8_t) + name_length;

    // Add space for cards if present
    if (decks[i].cards.ptr && decks[i].cards.size > 0) {
      total_size += decks[i].cards.size * CARD_BYTES;
    }
  }

  // Allocate buffer
  uint8_t *buf = new uint8_t[total_size];
  size_t offset = 0;

  // Store number of decks
  buf[offset++] = MAX_DECKS;

  // Second pass: write data to buffer
  for (size_t i = 0; i < MAX_DECKS; i++) {
    // Write card count (0 for empty decks)
    uint8_t card_count = 0;
    if (decks[i].cards.ptr && decks[i].cards.size > 0) {
      card_count = static_cast<uint8_t>(decks[i].cards.size);
    }
    buf[offset++] = card_count;

    // Check if name is blank
    size_t name_length = strnlen(decks[i].name, MAX_DECK_NAME_LENGTH);
    bool is_blank = (name_length == 0);
    char default_name[16];
    const char *final_name;

    if (!is_blank) {
      // Check if name is only whitespace
      bool only_whitespace = true;
      for (size_t j = 0; j < name_length; j++) {
        if (!isspace(decks[i].name[j])) {
          only_whitespace = false;
          break;
        }
      }
      is_blank = only_whitespace;
    }

    // Prepare name data
    if (is_blank) {
      sprintf(default_name, "Deck %u", i + 1);
      name_length = strlen(default_name);
      final_name = default_name;
    } else {
      final_name = decks[i].name;
    }

    // Write name length
    buf[offset++] = static_cast<uint8_t>(name_length);

    // Write name
    memcpy(buf + offset, final_name, name_length);
    offset += name_length;

    // Write card data if present
    if (card_count > 0) {
      memcpy(buf + offset, decks[i].cards.ptr, card_count * CARD_BYTES);
      offset += card_count * CARD_BYTES;
    }
  }

  return fat_ptr<uint8_t>{buf, total_size};
}

fat_ptr<deck_t> deserialize_decks(const fat_ptr<uint8_t> &bytes) {
  if (!bytes.ptr || bytes.size == 0) {
    return fat_ptr<deck_t>{nullptr, 0};
  }

  // Validate minimum buffer size (at least 1 byte for deck count)
  if (bytes.size < sizeof(uint8_t)) {
    return fat_ptr<deck_t>{nullptr, 0};
  }

  deck_t *decks = new deck_t[MAX_DECKS];
  size_t offset = 0;

  // Read number of decks
  uint8_t num_decks = bytes.ptr[offset++];
  if (num_decks > MAX_DECKS)
    num_decks = MAX_DECKS;

  // Initialize all decks with defaults
  for (size_t i = 0; i < MAX_DECKS; i++) {
    sprintf(decks[i].name, "Deck %u", i + 1);
    decks[i].cards = fat_ptr<card_t>{nullptr, 0};
  }

  // Read deck data
  for (size_t i = 0; i < num_decks && offset < bytes.size; i++) {
    // Read card count
    if (offset >= bytes.size)
      break;
    uint8_t card_count = bytes.ptr[offset++];

    // Read name length
    if (offset >= bytes.size)
      break;
    uint8_t name_length = bytes.ptr[offset++];

    // Validate remaining buffer size for name
    if (offset + name_length > bytes.size)
      break;

    // Read name if present
    if (name_length > 0) {
      memcpy(decks[i].name, bytes.ptr + offset, name_length);
      decks[i].name[name_length] = '\0'; // Ensure null termination
      offset += name_length;
    }

    // Read card data if present
    if (card_count > 0) {
      size_t cards_size = card_count * CARD_BYTES;

      // Validate remaining buffer size for cards
      if (offset + cards_size > bytes.size)
        break;

      card_t *cards = new card_t[card_count];
      memcpy(cards, bytes.ptr + offset, cards_size);
      decks[i].cards = fat_ptr<card_t>{cards, card_count};
      offset += cards_size;
    }
  }

  return fat_ptr<deck_t>{decks, MAX_DECKS};
}

// Optimize serialize_user_cards by removing unnecessary copy
fat_ptr<uint8_t> serialize_user_cards(const fat_ptr<user_card> &cards) {
  if (is_fat_nullptr(cards)) {
    return fat_ptr<uint8_t>{nullptr, 0};
  }

  const size_t data_size = cards.size * sizeof(user_card);
  const size_t total_size = sizeof(uint8_t) + data_size;
  uint8_t *buffer = new uint8_t[total_size];

  buffer[0] = cards.size;
  // Single memcpy for all card data
  memcpy(buffer + 1, cards.ptr, data_size);

  return fat_ptr<uint8_t>{buffer, total_size};
}

fat_ptr<user_card> deserialize_user_cards(const fat_ptr<uint8_t> &bytes) {
  if (is_fat_nullptr(bytes) || bytes.size < sizeof(uint8_t)) {
    return fat_ptr<user_card>{nullptr, 0};
  }

  const size_t count = bytes.ptr[0];
  const size_t expected_size = sizeof(uint8_t) + (count * sizeof(user_card));

  // Verify buffer has enough data
  if (bytes.size < expected_size) {
    return fat_ptr<user_card>{nullptr, 0};
  }

  user_card *buffer = new user_card[count];
  memcpy(buffer, bytes.ptr + 1, count * sizeof(user_card));

  return fat_ptr<user_card>{buffer, count};
}

#endif