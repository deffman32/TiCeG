#include "fonts.h"

/* This contains the raw data for the font. */
static const unsigned char cherry_10_font_data[] = {
#include "cherry-10.inc"
};
static const unsigned char cherry_13_font_data[] = {
#include "cherry-13.inc"
};
static const unsigned char cherry_20_font_data[] = {
#include "cherry-20.inc"
};
static const unsigned char cherry_26_font_data[] = {
#include "cherry-26.inc"
};

/* However, C89 does not allow us to typecast a byte array into a
fontlib_font_t pointer directly, so we have to use a second statement to do it,
though helpfully we can at least do it in the global scope. */
const fontlib_font_t *cherry_10_font = (fontlib_font_t *)cherry_10_font_data;
const fontlib_font_t *cherry_13_font = (fontlib_font_t *)cherry_13_font_data;
const fontlib_font_t *cherry_20_font = (fontlib_font_t *)cherry_20_font_data;
const fontlib_font_t *cherry_26_font = (fontlib_font_t *)cherry_26_font_data;