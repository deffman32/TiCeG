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
#include "utils.h"
#include <compression.h>
#include <debug.h>
#include <fileioc.h>
#include <fontlibc.h>
#include <graphx.h>
#include <string.h>
#include <tice.h>

const char *CARD_NAMES[] = {"Test Card 1", "Test Card 2", "Test Card 3",
                            "Test Card 4"};

gfx_UninitedRLETSprite(battle_icon,
                       (battle_icon_width + 1) * battle_icon_height);
gfx_UninitedRLETSprite(trade_icon, (trade_icon_width + 1) * trade_icon_height);

const fontlib_font_t* DEFAULT_FONT = cherry_13_font;
const fontlib_load_options_t FONT_LOAD_OPTIONS = (fontlib_load_options_t) 0;

void begin() {
  zx0_Decompress(battle_icon, battle_icon_compressed);
  zx0_Decompress(trade_icon, trade_icon_compressed);
}

void graphics_begin() {
  gfx_Begin();
  gfx_SetDrawBuffer();

  gfx_SetPalette(global_palette, sizeof_global_palette, 0);

  gfx_SetTransparentColor(C_TRANSPARENT);

  gfx_FillScreen(C_WHITE);

  gfx_SetColor(C_BLACK);

  fontlib_SetFont(DEFAULT_FONT, FONT_LOAD_OPTIONS);
  fontlib_SetLineSpacing(1, 1);
  fontlib_SetTransparency(true);
  fontlib_SetColors(C_BLACK, C_WHITE);
}

void end() {}

bool step() {
  const uint8_t key = os_GetCSC();

  return key != sk_Clear;
}

uint24_t fontlib_DrawStringCentered(const char *str, unsigned int x, uint8_t y) {
  fontlib_SetCursorPosition(x - fontlib_GetStringWidth(str) / 2, y - fontlib_GetCurrentFontHeight() / 2);
  return fontlib_DrawString(str);
}

void draw() {
  gfx_FillScreen(0xC1);
  gfx_RLETSprite(trade_icon, (LCD_WIDTH - trade_icon_width) / 2,
                 (LCD_HEIGHT - trade_icon_height) / 2);
  gfx_RLETSprite(battle_icon, (LCD_WIDTH + battle_icon_width) / 2 + 7,
                 (LCD_HEIGHT - battle_icon_height) / 2);
  gfx_SetColor(0xc0);
  gfx_FillCircle((LCD_WIDTH + battle_icon_width) / 2 - 7 - 54 - 27, (LCD_HEIGHT - battle_icon_height) / 2 + 27, 27);
  gfx_SetColor(0xc3);
  gfx_Circle((LCD_WIDTH + battle_icon_width) / 2 - 7 - 54 - 27, (LCD_HEIGHT - battle_icon_height) / 2 + 27, 28);
  fontlib_SetFont(cherry_26_font, FONT_LOAD_OPTIONS);
  fontlib_DrawStringCentered("TICEG", LCD_WIDTH / 2, 15);
  fontlib_SetFont(DEFAULT_FONT, FONT_LOAD_OPTIONS);
  fontlib_DrawStringCentered("A TRADING CARD GAME FOR", LCD_WIDTH / 2, 40);
  fontlib_DrawStringCentered("THE TI 84 PLUS CE FAMILY", LCD_WIDTH / 2, 60);
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
