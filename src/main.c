#include <tice.h>
#include <fileioc.h>
#include <graphx.h>
#include <ti/getkey.h>
#include "gfx/gfx.h"
#include <string.h>
#include "fonts/fonts.h"
#include <math.h>

#define BLACK_INDEX 240

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

const char* CARD_NAMES[] = {
    "Test Card 1",
    "Test Card 2",
    "Test Card 3",
    "Test Card 4"
};

typedef struct
{
    int damage;
    int defense;
    int card_idx;
} card_t;

void limit_length(char* str, const size_t max_len)
{
    if (strlen(str) > max_len)
    {
        str[max_len] = '\0';
    }
}

uint24_t fontlib_DrawStringXY(const char* string, unsigned int x, uint8_t y)
{
    fontlib_SetCursorPosition(x, y);
    return fontlib_DrawString(string);
}

const int CARD_WIDTH = 80;
const int CARD_HEIGHT = 120;
const int STR_PADDING = 8;

void drawCard(const int center_x, const int center_y, const card_t card)
{
    gfx_SetColor(BLACK_INDEX);
    gfx_FillRectangle(center_x - CARD_WIDTH / 2, center_y - CARD_HEIGHT / 2, CARD_WIDTH, CARD_HEIGHT);
    gfx_SetColor(255);
    gfx_FillRectangle(center_x - CARD_WIDTH / 2 + 2, center_y - CARD_HEIGHT / 2 + 2, CARD_WIDTH - 4,
                      CARD_HEIGHT - 4);

    char* string = (char*)CARD_NAMES[card.card_idx];
    unsigned int str_width = fontlib_GetStringWidth(string);
    unsigned int i = strlen(string);
    while (str_width > CARD_WIDTH - STR_PADDING * 2)
    {
        i -= 1;
        string[i] = '\0';
        str_width = fontlib_GetStringWidth(string);
    }
    fontlib_DrawStringXY(string, center_x - CARD_WIDTH / 2 + STR_PADDING, center_y - CARD_HEIGHT / 2 + 6);
}

void drawHand(const card_t* hand, const size_t handCount)
{
    const int CARD_COUNT = MIN(handCount, 5);

    int x = CARD_WIDTH / 2;
    int y = LCD_HEIGHT + 20;

    for (int i = CARD_COUNT - 1; i >= 0; i--)
    {
        drawCard(x, y, hand[i]);
        x += 40;
        y -= 2;
    }
}

void begin()
{
}

void graphics_begin()
{
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(global_palette, sizeof_global_palette, 0);

    gfx_SetTransparentColor(254); // TODO: Pick a good transparent color index

    gfx_FillScreen(255);

    gfx_SetColor(BLACK_INDEX);

    fontlib_SetFont(drsans_font, 0);
    fontlib_SetLineSpacing(2, 2);
    fontlib_SetTransparency(true);
    fontlib_SetColors(BLACK_INDEX, 0xFF);
}

void end()
{
}

bool step()
{
    return os_GetCSC() != k_Clear;
}

const card_t hand[] = {
    {
        10,
        10,
        0
    },
    {
        9,
        9,
        1
    },
    {
        10,
        10,
        2
    },
    {
        10,
        10,
        1
    },
    {
        10,
        10,
        0
    }
};

void draw()
{
    drawHand(hand, sizeof(hand) / sizeof(card_t));
}

int main()
{
    begin();

    graphics_begin();

    while (step())
    {
        draw();
        gfx_SwapDraw();
    }

    gfx_End();
    end();
}
