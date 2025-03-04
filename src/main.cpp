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
#include <debug.h>
#include <fileioc.h>
#include <fontlibc.h>
#include <graphx.h>
#include <string.h>
#include <tice.h>

const char *CARD_NAMES[] = {"Test Card 1", "Test Card 2", "Test Card 3",
                            "Test Card 4"};

void begin() {}

void graphics_begin() {
  gfx_Begin();
  gfx_SetDrawBuffer();

  gfx_SetPalette(global_palette, sizeof_global_palette, 0);

  gfx_SetTransparentColor(C_TRANSPARENT);

  gfx_FillScreen(C_WHITE);

  gfx_SetColor(C_BLACK);

  fontlib_SetFont(drsans_06_font, (fontlib_load_options_t)0);
  fontlib_SetLineSpacing(2, 2);
  fontlib_SetTransparency(true);
  fontlib_SetColors(C_BLACK, C_WHITE);
}

void end() {}

bool step() {
  const uint8_t key = os_GetCSC();

  return key != sk_Clear;
}

void draw() {
  gfx_FillScreen(0xC1);
  gfx_RLETSprite(TradeSymbol, LCD_WIDTH / 2 - TradeSymbol_width / 2,
                 LCD_HEIGHT / 2 - TradeSymbol_height / 2);
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
