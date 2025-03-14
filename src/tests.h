#include "serialize.h"
#include "utils.h"

void test_serialize() {
#ifndef NDEBUG
  // Test deck serialization and deserialization
  dbg_printf("Testing deck serialization...\n");

  // Create test cards
  card_t test_cards[3] = {
      {10, 5, 0}, // damage: 10, defense: 5, card_idx: 0
      {8, 7, 1},  // damage: 8, defense: 7, card_idx: 1
      {12, 3, 2}  // damage: 12, defense: 3, card_idx: 2
  };

  // Create test decks
  deck_t test_decks[10];
  for (int i = 0; i < 10; i++) {
    if (i < 3) {
      // First three decks have cards
      test_decks[i] = {{new card_t[i + 1], (size_t)i + 1}, {}};

      // Set meaningful deck names
      switch (i) {
      case 0:
        strcpy(test_decks[i].name, "Single Card");
        break;
      case 1:
        strcpy(test_decks[i].name, "Double Trouble");
        break;
      case 2:
        strcpy(test_decks[i].name, "Triple Threat");
        break;
      }

      // Copy i+1 cards to this deck
      for (int j = 0; j < i + 1; j++) {
        test_decks[i].cards.ptr[j] = test_cards[j % 3];
        test_decks[i].cards.size = i + 1;
      }

      dbg_printf("Created deck %d with %d cards, name: %s\n", i, i + 1,
                 test_decks[i].name);
    } else {
      // Rest are empty decks
      test_decks[i] = EMPTY_DECK;
    }
  }

  dbg_printf("%d\n", test_decks[0].cards.size);

  // Serialize decks
  fat_ptr<uint8_t> serialized = serialize_decks(test_decks);
  dbg_printf("Serialized to %d bytes\n", serialized.size);

  // Debug - print raw bytes of serialized data
  dbg_printf("Raw serialized data: ");
  for (size_t i = 0; i < serialized.size; i++) {
    dbg_printf("%02X ", serialized.ptr[i]);
  }
  dbg_printf("\n");

  fat_ptr<deck_t> deserialized = deserialize_decks(serialized);

  // Verify deserialization worked
  if (deserialized.size == 10) {
    dbg_printf("Deserialization successful, got %d decks\n", deserialized.size);

    for (int i = 0; i < 10; i++) {
      if (deserialized.ptr[i].cards.size > 0) {
        dbg_printf("Deck %d (name: %s) has %d cards\n", i,
                   deserialized.ptr[i].name, deserialized.ptr[i].cards.size);

        // Verify name matches the original (for first 3 decks)
        if (i < 3) {
          if (strcmp(deserialized.ptr[i].name, test_decks[i].name) == 0) {
            dbg_printf("  Name verification passed!\n");
          } else {
            dbg_printf("  Name verification FAILED! Expected: %s, Got: %s\n",
                       test_decks[i].name, deserialized.ptr[i].name);
          }
        }

        // Print details of each card in this deck
        dbg_printf("  Cards in deck %d:\n", i);
        for (size_t j = 0; j < deserialized.ptr[i].cards.size; j++) {
          card_t card = deserialized.ptr[i].cards.ptr[j];
          dbg_printf("    Card %d: damage=%d, defense=%d, card_idx=%d\n", j,
                     card.damage, card.defense, card.card_idx);
        }
      } else {
        dbg_printf("Deck %d is empty\n", i);
      }
    }
  } else {
    dbg_printf("Deserialization failed!\n");
  }

  // Clean up
  delete[] serialized.ptr;

  // Clean up test decks
  for (int i = 0; i < 3; i++) {
    delete[] test_decks[i].cards.ptr;
  }

  // Clean up deserialized decks
  for (size_t i = 0; i < deserialized.size; i++) {
    if (!is_fat_nullptr(deserialized.ptr[i].cards)) {
      delete[] deserialized.ptr[i].cards.ptr;
    }
  }
  delete[] deserialized.ptr;
#endif
}