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

gfx_UninitedRLETSprite(battle_icon,
                       (battle_icon_width + 1) * battle_icon_height);
gfx_UninitedRLETSprite(trade_icon, (trade_icon_width + 1) * trade_icon_height);
gfx_UninitedRLETSprite(decks_icon, (decks_icon_width + 1) * decks_icon_height);

typedef enum {
  FONT_CHERRY_10,
  FONT_CHERRY_13,
  FONT_CHERRY_20,
  FONT_CHERRY_26
} font_type_t;

constexpr font_type_t DEFAULT_FONT = FONT_CHERRY_13;

font_type_t CURRENT_FONT = FONT_CHERRY_13;

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
    return fontlib_SetFont(font_data, (fontlib_load_options_t)0);
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

  fat_ptr<deck_t> decks = deserialize_decks(buf);

  for (size_t i = 0; i < decks.size; i++) {
    global_state.decks[i] = decks.ptr[i];
  };

  dbg_printf("User Data Size: %d\n", size);

  ti_SetArchiveStatus(true, handle);

  ti_Close(handle);

#if ENABLE_TESTS
#include "tests.h"
  test_serialize();
#endif
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

int selectedMenuIdx = 1;

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
    fontlib_SetForegroundColor(selectedMenuIdx == i ? 0xC3 : 0xC2);
    fontlib_DrawStringCentered(menu_items[i], x_positions[i],
                               (LCD_HEIGHT + battle_icon_height) / 2 + 7);
  }
  fontlib_SetForegroundColor(C_BLACK);
}

void drawDeckScreen() {
  gfx_FillScreen(0xC3);
  ticeg_SetFont(FONT_CHERRY_20);

  gfx_SetColor(0xC1);
  gfx_FillRoundedRect(0, 0, LCD_WIDTH / 2, 40, 8);
  fontlib_SetForegroundColor(C_BLACK);
  fontlib_DrawStringCentered("DECKS", LCD_WIDTH / 4, 20);
  gfx_FillRoundedRect(LCD_WIDTH / 2 + 5, 0, LCD_WIDTH, 40, 8);
  fontlib_DrawStringCentered("YOUR CARDS", LCD_WIDTH / 4 * 3, 20);
}

void draw() {
  switch (global_state.current_screen) {
  case M_HOME:
    drawHomeScreen();
    break;
  case M_DECKS:
    drawDeckScreen();
    break;
  default:
    break;
  }
}

bool step() {
  const uint8_t key = os_GetCSC();

  if (global_state.current_screen == M_HOME) {
  }

  if (global_state.current_screen == M_HOME) {
    switch (key) {
    case sk_Left:
      selectedMenuIdx = MAX(selectedMenuIdx - 1, 0);
      break;
    case sk_Right:
      selectedMenuIdx = MIN(selectedMenuIdx + 1, 2);
      break;
    case sk_Enter:
      switch (selectedMenuIdx) {
      case 0:
        global_state.current_screen = M_DECKS;
        break;
      };
      break;
    }
  } else if (global_state.current_screen == M_DECKS) {
    switch (key) {
    case sk_Mode:
      global_state.current_screen = M_HOME;
      selectedMenuIdx = 1;
    }
  }

  return key != sk_Clear;
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
