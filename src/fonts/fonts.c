#include "fonts.h"

/* This contains the raw data for the font. */
static const unsigned char drsans_06_font_data[] = {
#include "drsans-06.inc"
};

static const unsigned char drsans_09_font_data[] = {
#include "drsans-09.inc"
};

/* However, C89 does not allow us to typecast a byte array into a
fontlib_font_t pointer directly, so we have to use a second statement to do it,
though helpfully we can at least do it in the global scope. */
const fontlib_font_t *drsans_06_font = (fontlib_font_t *)drsans_06_font_data;
const fontlib_font_t *drsans_09_font = (fontlib_font_t *)drsans_09_font_data;