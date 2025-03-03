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

typedef enum : int {
  M_DEFAULT,
  M_HAND,
  M_END, // Used to tell the amount of areas
} menu_area;

typedef struct {
  menu_area area;
  card_t *hand_cards;
  int hand_cards_count;
  int selected_card;
} game_state_t;

game_state_t game_state;

const int CARD_WIDTH = 80;
const int CARD_HEIGHT = 120;
const int STR_PADDING = 8;

void drawCard(const int center_x, const int center_y, const card_t card) {
  gfx_SetColor(C_BLACK);
  gfx_FillRectangle(center_x - CARD_WIDTH / 2, center_y - CARD_HEIGHT / 2,
                    CARD_WIDTH, CARD_HEIGHT);
  gfx_SetColor(C_WHITE);
  gfx_FillRectangle(center_x - CARD_WIDTH / 2 + 2,
                    center_y - CARD_HEIGHT / 2 + 2, CARD_WIDTH - 4,
                    CARD_HEIGHT - 4);

  char *string = (char *)CARD_NAMES[card.card_idx];
  unsigned int str_width = fontlib_GetStringWidth(string);
  unsigned int i = strlen(string);
  while (str_width > CARD_WIDTH - STR_PADDING * 2) {
    i -= 1;
    string[i] = '\0';
    str_width = fontlib_GetStringWidth(string);
  }
  fontlib_SetForegroundColor(C_BLACK);
  fontlib_DrawStringXY(string, center_x - CARD_WIDTH / 2 + STR_PADDING,
                       center_y - CARD_HEIGHT / 2 + 6);
}

void drawBigCard(const int center_x, const int center_y, const card_t card) {
  const int BIG_CARD_WIDTH = 120;
  const int BIG_CARD_HEIGHT = 180;
  gfx_SetColor(C_BLACK);
  gfx_FillRectangle(center_x - BIG_CARD_WIDTH / 2,
                    center_y - BIG_CARD_HEIGHT / 2, BIG_CARD_WIDTH,
                    BIG_CARD_HEIGHT);
  gfx_SetColor(C_WHITE);
  gfx_FillRectangle(center_x - BIG_CARD_WIDTH / 2 + 2,
                    center_y - BIG_CARD_HEIGHT / 2 + 2, BIG_CARD_WIDTH - 4,
                    BIG_CARD_HEIGHT - 4);

  char *string = (char *)CARD_NAMES[card.card_idx];
  unsigned int str_width = fontlib_GetStringWidth(string);
  unsigned int i = strlen(string);
  while (str_width > BIG_CARD_WIDTH - STR_PADDING * 2) {
    i -= 1;
    string[i] = '\0';
    str_width = fontlib_GetStringWidth(string);
  }
  fontlib_SetForegroundColor(C_BLACK);
  fontlib_DrawStringXY(string, center_x - BIG_CARD_WIDTH / 2 + STR_PADDING,
                       center_y - BIG_CARD_HEIGHT / 2 + 6);
  fontlib_SetFont(drsans_09_font, (fontlib_load_options_t)0);
  fontlib_SetForegroundColor(C_DARK_RED);
  char buf[4];
  ucta(card.damage, buf);
  const int damage_x = center_x - BIG_CARD_WIDTH / 2 + STR_PADDING + 20;
  const int damage_y =
      center_y + BIG_CARD_HEIGHT / 2 - 6 - fontlib_GetCurrentFontHeight();
  fontlib_DrawStringXY(buf, damage_x, damage_y);
  fontlib_SetForegroundColor(C_LIGHT_BLUE);
  ucta(card.defense, buf);
  const int defense_x =
      center_x + BIG_CARD_WIDTH / 2 - STR_PADDING - fontlib_GetStringWidth(buf);
  const int defense_y =
      center_y + BIG_CARD_HEIGHT / 2 - 6 - fontlib_GetCurrentFontHeight();
  fontlib_DrawStringXY(buf, defense_x, defense_y);
  fontlib_SetFont(drsans_06_font, (fontlib_load_options_t)0);
  gfx_Sprite(fist, damage_x - 20, damage_y);
  gfx_Sprite(fist, defense_x - 20, damage_y);
}

void drawHand(const card_t *hand, const size_t handCount) {
  const int CARD_COUNT = MIN(handCount, 5);

  int x = CARD_WIDTH / 2;
  int y = LCD_HEIGHT + 20;

  for (int i = CARD_COUNT - 1; i >= 0; i--) {
    drawCard(x, y, hand[i]);
    x += 40;
    y -= 2;
  }
}

card_t default_hand[] = {
    {10, 10, 0}, {9, 9, 1}, {10, 10, 2}, {10, 10, 1}, {10, 10, 0}};

void begin() {
  const game_state_t new_game_state = {
      M_DEFAULT, default_hand, (int)(sizeof(default_hand) / sizeof(card_t)), 0};
  game_state = new_game_state;
}

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

  switch (key) {
  case sk_Up:
    game_state.area = static_cast<menu_area>(MAX(game_state.area - 1, 0));
    break;
  case sk_Down:
    game_state.area =
        static_cast<menu_area>(MIN(game_state.area + 1, M_END - 1));
    break;
  case sk_Left:
    game_state.selected_card = MAX(game_state.selected_card - 1, 0);
    break;
  case sk_Right:
    game_state.selected_card =
        MIN(game_state.selected_card + 1, game_state.hand_cards_count - 1);
    break;
  default:
    break;
  }

  return key != sk_Clear;
}

void draw() {
  gfx_FillScreen(C_WHITE);
  drawHand(game_state.hand_cards, game_state.hand_cards_count);
  if (game_state.area == M_HAND) {
    drawBigCard(LCD_WIDTH / 2, LCD_HEIGHT / 2 - 20,
                game_state.hand_cards[game_state.selected_card]);
  }
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
