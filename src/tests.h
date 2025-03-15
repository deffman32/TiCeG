#include "serialize.h"
#include "utils.h"

void test_serialize() {
#ifndef NDEBUG

  // Create test cards
  card_t test_cards[3] = {0, 1, 2};

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
    } else {
      // Rest are empty decks
      test_decks[i] = EMPTY_DECK;
    }
  }

  // Serialize decks
  fat_ptr<uint8_t> serialized = serialize_decks(test_decks);

  fat_ptr<deck_t> deserialized = deserialize_decks(serialized);

  // Verify deserialization worked - simplified to pass/fail
  bool test_passed = true;

  // Check if we got the right number of decks back
  if (deserialized.size != 10) {
    test_passed = false;
  } else {
    // Check the first 3 decks that should have cards
    for (size_t i = 0; i < 3 && test_passed; i++) {
      // Check deck name
      if (strcmp(deserialized.ptr[i].name, test_decks[i].name) != 0) {
        test_passed = false;
        break;
      }

      // Check number of cards
      if (deserialized.ptr[i].cards.size != i + 1) {
        test_passed = false;
        break;
      }

      // Verify each card's data
      for (size_t j = 0; j < deserialized.ptr[i].cards.size && test_passed;
           j++) {
        card_t orig_card = test_cards[j % 3];
        card_t deser_card = deserialized.ptr[i].cards.ptr[j];

        if (deser_card != orig_card) {
          test_passed = false;
          break;
        }
      }
    }

    // Check remaining decks are empty
    for (int i = 3; i < 10 && test_passed; i++) {
      if (deserialized.ptr[i].cards.size != 0) {
        test_passed = false;
        break;
      }
    }
  }

  // Print test result
  if (test_passed) {
    dbg_printf("Serialization test: PASSED\n");
  } else {
    dbg_printf("Serialization test: FAILED\n");
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

  // Test user_card serialization
  const size_t test_user_cards_count = 4;
  user_card *test_user_cards = new user_card[test_user_cards_count];

  // Initialize test user cards
  for (size_t i = 0; i < test_user_cards_count; i++) {
    test_user_cards[i].card = i;
    test_user_cards[i].count = (i + 1) * 2; // 2, 4, 6, 8 cards of each type
  }

  // Create fat_ptr to test user cards
  fat_ptr<user_card> user_cards_ptr = {test_user_cards, test_user_cards_count};

  // Serialize user cards
  fat_ptr<uint8_t> user_cards_serialized = serialize_user_cards(user_cards_ptr);

  // Deserialize user cards
  fat_ptr<user_card> user_cards_deserialized =
      deserialize_user_cards(user_cards_serialized);

  // Verify deserialization worked
  bool user_cards_test_passed = true;

  // Check if we got the right number of user cards back
  if (user_cards_deserialized.size != test_user_cards_count) {
    user_cards_test_passed = false;
    dbg_printf("User cards count mismatch: expected %d, got %d\n",
               test_user_cards_count, user_cards_deserialized.size);
  } else {
    // Verify each user card's data
    for (size_t i = 0; i < test_user_cards_count && user_cards_test_passed;
         i++) {
      if (user_cards_deserialized.ptr[i].card != test_user_cards[i].card) {
        user_cards_test_passed = false;
        dbg_printf("User card %d ID mismatch: expected %d, got %d\n", i,
                   test_user_cards[i].card,
                   user_cards_deserialized.ptr[i].card);
        break;
      }

      if (user_cards_deserialized.ptr[i].count != test_user_cards[i].count) {
        user_cards_test_passed = false;
        dbg_printf("User card %d count mismatch: expected %d, got %d\n", i,
                   test_user_cards[i].count,
                   user_cards_deserialized.ptr[i].count);
        break;
      }
    }
  }

  // Print test result
  if (user_cards_test_passed) {
    dbg_printf("User card serialization test: PASSED\n");
  } else {
    dbg_printf("User card serialization test: FAILED\n");
  }

  // Clean up user card test data
  delete[] test_user_cards;
  delete[] user_cards_serialized.ptr;
  delete[] user_cards_deserialized.ptr;
#endif
}