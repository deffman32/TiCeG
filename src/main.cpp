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

const fontlib_font_t *DEFAULT_FONT = cherry_13_font;
#define FONT_LOAD_OPTIONS ((fontlib_load_options_t)0)
#define RESET_FONT() fontlib_SetFont(DEFAULT_FONT, FONT_LOAD_OPTIONS)

void begin() {
  state_t state = {M_HOME};
  global_state = state;
  zx0_Decompress(battle_icon, battle_icon_compressed);
  zx0_Decompress(trade_icon, trade_icon_compressed);
  zx0_Decompress(decks_icon, decks_icon_compressed);
}

void graphics_begin() {
  gfx_Begin();
  gfx_SetDrawBuffer();

  gfx_SetPalette(global_palette, sizeof_global_palette, 0);

  gfx_SetTransparentColor(C_TRANSPARENT);

  gfx_FillScreen(C_WHITE);

  gfx_SetColor(C_BLACK);

  RESET_FONT();
  fontlib_SetLineSpacing(1, 1);
  fontlib_SetTransparency(true);
  fontlib_SetColors(C_BLACK, C_WHITE);
}

void end() {}

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
  fontlib_SetFont(cherry_26_font, FONT_LOAD_OPTIONS);
  fontlib_DrawStringCentered("TICEG", LCD_WIDTH / 2, 15);
  RESET_FONT();
  fontlib_DrawStringCentered("A TRADING CARD GAME FOR", LCD_WIDTH / 2, 40);
  fontlib_DrawStringCentered("THE TI 84 PLUS CE FAMILY", LCD_WIDTH / 2, 60);
  fontlib_SetFont(cherry_10_font, FONT_LOAD_OPTIONS);

  const char *menu_items[] = {"DECKS", "BATTLE", "TRADE"};
  const int x_positions[] = {LCD_WIDTH / 2 - battle_icon_width - 7,
                             LCD_WIDTH / 2,
                             LCD_WIDTH / 2 + battle_icon_width + 7};
  for (int i = 0; i < 3; i++) {
    fontlib_SetForegroundColor(selectedMenuIdx == i ? 0xC3 : 0xC2);
    fontlib_DrawStringCentered(menu_items[i], x_positions[i],
                               (LCD_HEIGHT + battle_icon_height) / 2 + 7);
  }
  RESET_FONT();
  fontlib_SetForegroundColor(C_BLACK);
}

void drawDeckScreen() {
  gfx_FillScreen(0xC1);
  fontlib_SetForegroundColor(0xC3);
  fontlib_SetCursorPosition(10, 10);
  fontlib_SetFont(cherry_20_font, FONT_LOAD_OPTIONS);
  fontlib_DrawString("DECKS");
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
  case sk_Mode:
    global_state.current_screen = M_HOME;
    selectedMenuIdx = 1;
  default:
    break;
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
