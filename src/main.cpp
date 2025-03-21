#define ENABLE_TESTS false

// Just for debug purposes
#define RESET_DATA false

#ifndef __INT24_TYPE__
#define __INT24_TYPE__
#endif
#ifndef __UINT24_TYPE__
#define __UINT24_TYPE__
#endif
#include "card.h"
#include "colors.h"
#include "fonts/fonts.h"
#include "gfx/gfx.h"
#include "graphics.h"
#include "serialize.h"
#include "state.h"
#include "tests.h"
#include "utils.h"
#include <compression.h>
#include <debug.h>
#include <fileioc.h>
#include <fontlibc.h>
#include <graphx.h>
#include <string.h>
#include <tice.h>

state_t global_state;

gfx_UninitedRLETSprite(battle_icon, battle_icon_size);
gfx_UninitedRLETSprite(trade_icon, trade_icon_size);
gfx_UninitedRLETSprite(decks_icon, decks_icon_size);

gfx_UninitedRLETSprite(left_arrow, left_arrow_size);
gfx_UninitedRLETSprite(right_arrow, right_arrow_size);

typedef enum {
  FONT_CHERRY_10,
  FONT_CHERRY_13,
  FONT_CHERRY_20,
  FONT_CHERRY_26
} font_type_t;

constexpr font_type_t DEFAULT_FONT = FONT_CHERRY_13;

font_type_t CURRENT_FONT = FONT_CHERRY_13;
uint8_t CURRENT_FONT_HEIGHT = 13;

__attribute__((always_inline)) inline bool ticeg_SetFont(font_type_t font) {
  if (CURRENT_FONT != font) {
    const fontlib_font_t *font_data = NULL;
    switch (font) {
    case FONT_CHERRY_10:
      font_data = cherry_10_font;
      break;
    case FONT_CHERRY_13:
      font_data = cherry_13_font;
      break;
    case FONT_CHERRY_20:
      font_data = cherry_20_font;
      break;
    case FONT_CHERRY_26:
      font_data = cherry_26_font;
      break;
    default:
      return false;
    }
    CURRENT_FONT = font;
    bool ret = fontlib_SetFont(font_data, (fontlib_load_options_t)0);
    CURRENT_FONT_HEIGHT = fontlib_GetCurrentFontHeight();
    return ret;
  }
  return true;
}

__attribute__((always_inline)) inline bool ticeg_ResetFont() {
  return ticeg_SetFont(DEFAULT_FONT);
}

const char *DATA_FILE_NAME = "TiCeG Data";

void begin() {
  state_t state = {M_HOME, {[0 ... 9] = EMPTY_DECK}, {}};
  global_state = state;
  zx0_Decompress(battle_icon, battle_icon_compressed);
  zx0_Decompress(trade_icon, trade_icon_compressed);
  zx0_Decompress(decks_icon, decks_icon_compressed);
  zx0_Decompress(left_arrow, left_arrow_compressed);
  zx0_Decompress(right_arrow, right_arrow_compressed);

  dbg_printf("Importing data...\n");

#if RESET_DATA
  ti_Delete(DATA_FILE_NAME);
#endif

  uint8_t handle;

  if ((handle = ti_Open(DATA_FILE_NAME, "r+")) == 0) {
    if ((handle = ti_Open(DATA_FILE_NAME, "w+")) == 0) {
      dbg_printf("Error reading data!");
      exit(0);
    }
    // Initialize with empty data
    dbg_printf("Creating new data file with empty data\n");
    fat_ptr<uint8_t> deck_data = serialize_decks(global_state.decks);
    fat_ptr<uint8_t> user_card_data =
        serialize_user_cards({global_state.user_cards, 4});

    dbg_printf("Initial deck data size: %d bytes\n", deck_data.size);
    dbg_printf("Initial user card data size: %d bytes\n", user_card_data.size);

    // Write both deck and user card data
    ti_Write(deck_data.ptr, 1, deck_data.size, handle);
    ti_Write(user_card_data.ptr, 1, user_card_data.size, handle);

    // Free memory
    delete[] deck_data.ptr;
    if (user_card_data.ptr)
      delete[] user_card_data.ptr;
  }

  uint16_t total_size = ti_GetSize(handle);
  dbg_printf("Total file size: %d bytes\n", total_size);
  fat_ptr<uint8_t> buf = {new uint8_t[total_size], total_size};

  ti_Read(buf.ptr, sizeof(uint8_t), total_size, handle);

  ti_SetArchiveStatus(true, handle);
  ti_Close(handle);

  // First, deserialize deck data
  fat_ptr<deck_t> decks = deserialize_decks(buf);
  dbg_printf("Deserialized %d decks\n", decks.size);
  for (size_t i = 0; i < decks.size; i++) {
    global_state.decks[i] = decks.ptr[i];
    dbg_printf("Deck %d: '%s' with %d cards\n", i, global_state.decks[i].name,
               global_state.decks[i].cards.size);
  }

  // Calculate offset of user cards data
  size_t deck_data_size = 0;

  // Calculate the size of deck data (1 byte deck count + deck data)
  if (decks.ptr && decks.size > 0) {
    // Calculate deck data size - we need this to offset into the buffer for
    // user cards Include header (1 byte) and iterate through each deck to add
    // up sizes
    deck_data_size = 1; // Start with header byte
    for (size_t i = 0; i < MAX_DECKS; i++) {
      // Add 1 byte for card count
      deck_data_size += 1;
      // Add 1 byte for name length + actual name length
      size_t name_length = strnlen(decks.ptr[i].name, MAX_DECK_NAME_LENGTH);
      if (name_length == 0) {
        // Default name if blank
        char default_name[16];
        sprintf(default_name, "Deck %u", i + 1);
        name_length = strlen(default_name);
      }
      deck_data_size += 1 + name_length;
      // Add card data size
      if (decks.ptr[i].cards.ptr && decks.ptr[i].cards.size > 0) {
        deck_data_size += decks.ptr[i].cards.size * CARD_BYTES;
      }
    }
    dbg_printf("Calculated deck data size: %d bytes\n", deck_data_size);
  }

  // Clean up the deck pointer
  if (decks.ptr)
    delete[] decks.ptr;

  // Now deserialize user cards if there's data left
  dbg_printf("Looking for user cards data at offset %d\n", deck_data_size);
  if (total_size > deck_data_size) {
    fat_ptr<uint8_t> user_cards_buf = {buf.ptr + deck_data_size,
                                       total_size - deck_data_size};

    dbg_printf("User cards buffer size: %d bytes\n", user_cards_buf.size);
    dbg_printf("First byte (card count): %d\n", user_cards_buf.ptr[0]);

    fat_ptr<user_card> data = deserialize_user_cards(user_cards_buf);
    dbg_printf("Deserialized %d user cards\n", data.size);

    if (data.size > 0) {
      memcpy(global_state.user_cards, data.ptr, data.size * sizeof(user_card));
      dbg_printf("Copied user cards to global state\n");

      // Print debug info for the first few cards
      for (size_t i = 0; i < data.size && i < 3; i++) {
        dbg_printf("Card %d: ID=%d, Count=%d\n", i, data.ptr[i].card,
                   data.ptr[i].count);
      }

      // Clean up
      delete[] data.ptr;
    } else {
      dbg_printf("No user cards found in data\n");
    }
  } else {
    dbg_printf("No user cards section found in file\n");
  }

  // Clean up the buffer
  delete[] buf.ptr;

  dbg_printf("User Data Size: %d\n", total_size);

  if (ENABLE_TESTS) {
    dbg_printf("\nRUNNING TESTS\n");
    test_serialize();
  }
}

void graphics_begin() {
  gfx_Begin();
  gfx_SetDrawBuffer();

  gfx_SetPalette(global_palette, sizeof_global_palette, 0);

  gfx_SetTransparentColor(C_TRANSPARENT);

  gfx_FillScreen(C_WHITE);

  gfx_SetColor(C_BLACK);

  ticeg_ResetFont();
  fontlib_SetLineSpacing(1, 1);
  fontlib_SetTransparency(true);
  fontlib_SetColors(C_BLACK, C_WHITE);
}

bool save_data() {
  dbg_printf("Saving data...\n");

  uint8_t handle;

  if ((handle = ti_Open(DATA_FILE_NAME, "w+")) == 0) {
    dbg_printf("Failed to open file for writing\n");
    return false;
  }

  fat_ptr<uint8_t> deck_buf = serialize_decks(global_state.decks);
  dbg_printf("Serialized deck data size: %d bytes\n", deck_buf.size);

  fat_ptr<uint8_t> user_cards_buf =
      serialize_user_cards({global_state.user_cards, 4});
  dbg_printf("Serialized user cards data size: %d bytes\n",
             user_cards_buf.size);

  // Debug info about user cards being saved
  dbg_printf("Saving %d user cards:\n",
             4);                   // Assuming 4 from the hardcoded size
  for (size_t i = 0; i < 4; i++) { // Print first 3 at most
    dbg_printf("Card %d: ID=%d, Count=%d\n", i, global_state.user_cards[i].card,
               global_state.user_cards[i].count);
  }

  // Write deck data
  ti_Write(deck_buf.ptr, sizeof(uint8_t), deck_buf.size, handle);

  // Write user cards data
  if (!is_fat_nullptr(user_cards_buf)) {
    ti_Write(user_cards_buf.ptr, sizeof(uint8_t), user_cards_buf.size, handle);
  }

  // Clean up
  if (deck_buf.ptr)
    delete[] deck_buf.ptr;
  if (user_cards_buf.ptr)
    delete[] user_cards_buf.ptr;

#ifndef NDEBUG
  uint16_t total_size = ti_GetSize(handle);
  dbg_printf("Total User Data Size: %d bytes\n", total_size);
#endif

  ti_SetArchiveStatus(true, handle);
  ti_Close(handle);

  return true;
}

void end() {
  if (!save_data()) {
    dbg_printf("Error writing data!");
  }
}

uint24_t fontlib_DrawStringCentered(const char *str, unsigned int x,
                                    uint8_t y) {
  fontlib_SetCursorPosition(x - fontlib_GetStringWidth(str) / 2,
                            y - fontlib_GetCurrentFontHeight() / 2);
  return fontlib_DrawString(str);
}

uint8_t SELECTED_MENU_IDX = 1;

void drawHomeScreen() {
  gfx_FillScreen(0xC1);
  gfx_RLETSprite(battle_icon, (LCD_WIDTH - battle_icon_width) / 2,
                 (LCD_HEIGHT - battle_icon_height) / 2);
  gfx_RLETSprite(trade_icon, (LCD_WIDTH + trade_icon_width) / 2 + 7,
                 (LCD_HEIGHT - trade_icon_height) / 2);
  gfx_RLETSprite(decks_icon,
                 (LCD_WIDTH - decks_icon_width) / 2 - 7 - decks_icon_width,
                 (LCD_HEIGHT - decks_icon_height) / 2);
  ticeg_SetFont(FONT_CHERRY_26);
  fontlib_DrawStringCentered("TICEG", LCD_WIDTH / 2, 15);
  ticeg_SetFont(FONT_CHERRY_13);
  fontlib_DrawStringCentered("A TRADING CARD GAME FOR", LCD_WIDTH / 2, 40);
  fontlib_DrawStringCentered("THE TI 84 PLUS CE FAMILY", LCD_WIDTH / 2, 60);
  ticeg_SetFont(FONT_CHERRY_10);

  const char *menu_items[] = {"DECKS", "BATTLE", "TRADE"};
  const int x_positions[] = {LCD_WIDTH / 2 - battle_icon_width - 7,
                             LCD_WIDTH / 2,
                             LCD_WIDTH / 2 + battle_icon_width + 7};
  for (int i = 0; i < 3; i++) {
    fontlib_SetForegroundColor(SELECTED_MENU_IDX == i ? 0xC3 : 0xC2);
    fontlib_DrawStringCentered(menu_items[i], x_positions[i],
                               (LCD_HEIGHT + battle_icon_height) / 2 + 7);
  }
  fontlib_SetForegroundColor(C_BLACK);
}

typedef enum {
  T_UNIT_CARD = 0,
  T_SUPPORT_CARD = 1,
  T_EFFECT_CARD = 2,
  T_DRAW_CARD = 3,
} card_type_t;

const char *serialize_card_type(card_type_t type) {
  const char *TYPE_NAMES[] = {"UNIT", "SUPPORT", "EFFECT", "DRAW"};
  return TYPE_NAMES[type];
}

card_type_t DECK_PANEL_CARD_TYPE = T_UNIT_CARD;
card_type_t CARDS_PANEL_CARD_TYPE = T_SUPPORT_CARD;

bool DECK_PANEL_SELECTED = true;

void drawDeckEditScreen() {
  gfx_FillScreen(0xC3);
  ticeg_SetFont(FONT_CHERRY_20);

  gfx_SetColor(0xC1);
  gfx_FillRoundedRect(0, 0, LCD_WIDTH / 2, 40, 8);
  fontlib_SetForegroundColor(C_BLACK);
  fontlib_DrawStringCentered("DECKS", LCD_WIDTH / 4, 20);
  gfx_FillRoundedRect(LCD_WIDTH / 2 + 5, 0, LCD_WIDTH, 40, 8);
  fontlib_DrawStringCentered("YOUR CARDS", LCD_WIDTH / 4 * 3 + 5, 20);
  gfx_SetColor(DECK_PANEL_SELECTED ? 0xC5 : 0xC1);
  gfx_FillRoundedRect(0, 45, LCD_WIDTH / 2, LCD_HEIGHT - 35, 8);
  gfx_SetColor(DECK_PANEL_SELECTED ? 0xC1 : 0xC5);
  gfx_FillRoundedRect(LCD_WIDTH / 2 + 5, 45, LCD_WIDTH, LCD_HEIGHT, 8);
  gfx_SetColor(0xC1);

  gfx_FillRoundedRect(0, LCD_HEIGHT - 30, LCD_WIDTH / 2, LCD_HEIGHT, 8);
  ticeg_SetFont(FONT_CHERRY_10);
  fontlib_SetCursorPosition(5, LCD_HEIGHT - 28);
  fontlib_DrawString("[MODE] Switch Panels");

  ticeg_SetFont(FONT_CHERRY_13);
  fontlib_DrawStringCentered(serialize_card_type(DECK_PANEL_CARD_TYPE),
                             LCD_WIDTH / 4, 55);

  gfx_RLETSprite(left_arrow, 5, 51);
  gfx_RLETSprite(right_arrow, LCD_WIDTH / 2 - 5 - right_arrow_width, 51);

  fontlib_DrawStringCentered(serialize_card_type(CARDS_PANEL_CARD_TYPE),
                             LCD_WIDTH / 4 * 3 + 5, 55);

  gfx_RLETSprite(left_arrow, LCD_WIDTH / 2 + 10, 51);
  gfx_RLETSprite(right_arrow, LCD_WIDTH - 5 - right_arrow_width, 51);
}

uint8_t SELECTED_DECK_INDEX = 0;

bool EDITING_DECK_NAME = false;

char CURRENT_INPUT[MAX_DECK_NAME_LENGTH] = {};
size_t CURRENT_INPUT_IDX = 0;

void drawDeckScreen() {
  gfx_FillScreen(0xC3);
  ticeg_SetFont(FONT_CHERRY_13);

  gfx_SetColor(0xC1);
  gfx_FillRoundedRect(0, 0, LCD_WIDTH, 40, 8);
  fontlib_SetForegroundColor(C_BLACK);
  fontlib_DrawStringCentered("WHICH DECK WOULD YOU LIKE TO EDIT?",
                             LCD_WIDTH / 2, 20);

  gfx_FillRoundedRect(0, 43, LCD_WIDTH, LCD_HEIGHT - 40, 8);
  gfx_FillRoundedRect(0, LCD_HEIGHT - 37, LCD_WIDTH, LCD_HEIGHT, 8);

  fontlib_SetCursorPosition(5, LCD_HEIGHT - 35);
  fontlib_DrawString("[STAT] Edit deck name");
  fontlib_SetCursorPosition(5, LCD_HEIGHT - 17);
  fontlib_DrawString("[DEL] Clear deck");

  for (size_t i = 0; i < MAX_DECKS; i++) {
    deck_t deck = global_state.decks[i];
    char *deck_name = deck.name;

    gfx_SetColor(0xC3);

    int y = 46 + i * 15 + CURRENT_FONT_HEIGHT / 2;

    if (i == SELECTED_DECK_INDEX) {
      gfx_FillRectangle(6, y, 6, 6);
    } else {
      gfx_Rectangle(6, y, 6, 6);
    }

    fontlib_SetCursorPosition(15, 46 + i * 15);

    if (deck_name[0] == '\0') {
      fontlib_DrawString("Deck ");
      fontlib_DrawString(digit_table[i + 1]);
    } else {
      fontlib_DrawString(deck_name);
    }
  }

  if (EDITING_DECK_NAME) {
    const int BORDER_WIDTH = 3;
    gfx_FillRoundedRect(LCD_WIDTH / 8, LCD_HEIGHT / 4, LCD_WIDTH / 8 * 7,
                        LCD_HEIGHT / 4 * 3, 8);
    gfx_SetColor(0xC5);
    gfx_FillRoundedRect(
        LCD_WIDTH / 8 + BORDER_WIDTH, LCD_HEIGHT / 4 + BORDER_WIDTH,
        LCD_WIDTH / 8 * 7 - BORDER_WIDTH, LCD_HEIGHT / 4 * 3 - BORDER_WIDTH, 8);

    char buf[50];

    sprintf(buf, "Renaming deck %u", SELECTED_DECK_INDEX + 1);

    fontlib_DrawStringCentered(buf, LCD_WIDTH / 2, LCD_HEIGHT / 2 - 40);

    fontlib_SetCursorPosition(LCD_WIDTH / 8 + BORDER_WIDTH + 5, LCD_HEIGHT / 2);
    fontlib_DrawString(CURRENT_INPUT);
  }
}

void draw() {
  switch (global_state.current_screen) {
  case M_HOME:
    drawHomeScreen();
    break;
  case M_DECKS:
    drawDeckScreen();
    break;
  case M_EDIT_DECK:
    drawDeckEditScreen();
    break;
  default:
    break;
  }
}

const char *chars = "\0\0\0\0\0\0\0\0\0\0\"WRMH\0\0\0\0VQLG\0\0\0ZUPKFC\0 "
                    "YTOJEB\0\0XSNIDA\0\0\0\0\0\0\0\0";

#define CLEAR_DECK_NAME(deck) memset(deck, '\0', MAX_DECK_NAME_LENGTH)

bool is_string_empty(char *str) {
  size_t len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    if (!isspace(str[i])) {
      return false;
    }
  }
  return true;
}

bool step() {
  const uint8_t key = os_GetCSC();

  bool shouldExit = key == sk_Clear && !EDITING_DECK_NAME;

  if (global_state.current_screen == M_HOME) {
    switch (key) {
    case sk_Left:
      if (SELECTED_MENU_IDX != 0) {
        SELECTED_MENU_IDX -= 1;
      }
      break;
    case sk_Right:
      if (SELECTED_MENU_IDX < 2) {
        SELECTED_MENU_IDX += 1;
      }
      break;
    case sk_Enter:
      switch (SELECTED_MENU_IDX) {
      case 0:
        global_state.current_screen = M_DECKS;
        SELECTED_DECK_INDEX = 0;
        break;
      };
      break;
    }
  } else if (global_state.current_screen == M_DECKS) {
    if (!EDITING_DECK_NAME) {
      switch (key) {
      case sk_Up:
        if (SELECTED_DECK_INDEX > 0) {
          SELECTED_DECK_INDEX -= 1;
        }
        break;
      case sk_Down:
        if (SELECTED_DECK_INDEX < MAX_DECKS - 1) {
          SELECTED_DECK_INDEX += 1;
        }
        break;
      case sk_Mode:
        global_state.current_screen = M_HOME;
        SELECTED_MENU_IDX = 1;
        SELECTED_DECK_INDEX = 0;
        break;
      case sk_Stat:
        EDITING_DECK_NAME = true;
        break;
      case sk_Del:
        global_state.decks[SELECTED_DECK_INDEX].cards = fat_nullptr<card_t>;
        CLEAR_DECK_NAME(global_state.decks[SELECTED_DECK_INDEX].name);
        break;
      case sk_Enter:
        global_state.current_screen = M_EDIT_DECK;
        break;
      }
    } else {
      if (key == sk_Clear) {
        CLEAR_DECK_NAME(CURRENT_INPUT);
        EDITING_DECK_NAME = false;
        CURRENT_INPUT_IDX = 0;
      } else if (key == sk_Enter) {
        char *ptr = global_state.decks[SELECTED_DECK_INDEX].name;
        CLEAR_DECK_NAME(ptr);
        memcpy(ptr, CURRENT_INPUT, CURRENT_INPUT_IDX);
        CLEAR_DECK_NAME(CURRENT_INPUT);
        EDITING_DECK_NAME = false;
        CURRENT_INPUT_IDX = 0;
        if (is_string_empty(ptr)) {
          CLEAR_DECK_NAME(ptr);
          char buf[16];
          sprintf(buf, "Deck %u", SELECTED_DECK_INDEX + 1);
          memcpy(ptr, buf, 16);
        }
      } else if (key == sk_Del) {
        CURRENT_INPUT[--CURRENT_INPUT_IDX] = '\0';
      } else {
        char c;
        if ((c = chars[key]) != '\0') {
          if (CURRENT_INPUT_IDX < MAX_DECK_NAME_LENGTH - 1) {
            CURRENT_INPUT[CURRENT_INPUT_IDX++] = c;
          }
        }
      }
    }
  } else if (global_state.current_screen == M_EDIT_DECK) {
    switch (key) {
    case sk_Mode:
      // Toggle
      DECK_PANEL_SELECTED ^= true;
      break;
    case sk_Left:
      if (DECK_PANEL_SELECTED) {
        DECK_PANEL_CARD_TYPE = static_cast<card_type_t>(
            (static_cast<int>(DECK_PANEL_CARD_TYPE) + 4 - 1) % 4);
      } else {
        CARDS_PANEL_CARD_TYPE = static_cast<card_type_t>(
            (static_cast<int>(CARDS_PANEL_CARD_TYPE) + 4 - 1) % 4);
      }
      break;
    case sk_Right:
      if (DECK_PANEL_SELECTED) {
        DECK_PANEL_CARD_TYPE = static_cast<card_type_t>(
            (static_cast<int>(DECK_PANEL_CARD_TYPE) + 1) % 4);
      } else {
        CARDS_PANEL_CARD_TYPE = static_cast<card_type_t>(
            (static_cast<int>(CARDS_PANEL_CARD_TYPE) + 1) % 4);
      }
      break;
    }
  }

  return !shouldExit;
}

int main() {
  begin();

  graphics_begin();

  while (step()) {
    draw();
    gfx_SwapDraw();
  }

  gfx_End();
  end();
}
