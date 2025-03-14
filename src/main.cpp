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

const char *CARD_NAMES[] = {"Test Card 1", "Test Card 2", "Test Card 3",
                            "Test Card 4"};

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
  state_t state = {M_HOME, {[0 ... 9] = EMPTY_DECK}};
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
    fat_ptr<uint8_t> data = serialize_decks(global_state.decks);
    ti_Write(data.ptr, 1, data.size, handle);
  }

  uint16_t size = ti_GetSize(handle);

  fat_ptr<uint8_t> buf = {new uint8_t[size], size};

  ti_Read(buf.ptr, sizeof(uint8_t), size, handle);

  ti_SetArchiveStatus(true, handle);

  ti_Close(handle);

  fat_ptr<deck_t> decks = deserialize_decks(buf);

  for (size_t i = 0; i < decks.size; i++) {
    global_state.decks[i] = decks.ptr[i];
  };

  dbg_printf("User Data Size: %d\n", size);

  if (ENABLE_TESTS) {
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
    return false;
  }

  fat_ptr<uint8_t> buf = serialize_decks(global_state.decks);

  ti_Write(buf.ptr, sizeof(uint8_t), buf.size, handle);

  dbg_printf("User Data Size: %d\n", buf.size);

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

void drawDeckEditScreen() {
  gfx_FillScreen(0xC3);
  ticeg_SetFont(FONT_CHERRY_20);

  gfx_SetColor(0xC1);
  gfx_FillRoundedRect(0, 0, LCD_WIDTH / 2, 40, 8);
  fontlib_SetForegroundColor(C_BLACK);
  fontlib_DrawStringCentered("DECKS", LCD_WIDTH / 4, 20);
  gfx_FillRoundedRect(LCD_WIDTH / 2 + 5, 0, LCD_WIDTH, 40, 8);
  fontlib_DrawStringCentered("YOUR CARDS", LCD_WIDTH / 4 * 3 + 5, 20);
  gfx_FillRoundedRect(0, 45, LCD_WIDTH / 2, LCD_HEIGHT - 35, 8);
  gfx_FillRoundedRect(LCD_WIDTH / 2 + 5, 45, LCD_WIDTH, LCD_HEIGHT, 8);

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
      if (!is_fat_nullptr(deck.cards)) {
        fontlib_DrawString("Deck ");
        fontlib_DrawString(digit_table[i + 1]);
      } else {
        fontlib_DrawString("<EMPTY>");
      }
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
      case sk_Enter:
        global_state.current_screen = M_EDIT_DECK;
        break;
      }
    } else {
      if (key == sk_Clear) {
        EDITING_DECK_NAME = false;
      }
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
