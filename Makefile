# ----------------------------
# Makefile Options
# ----------------------------

NAME = TiCeG
ICON = icon.png
DESCRIPTION = "A Trading Card Game (TCG) meant for the TI 84 Plus CE family"
COMPRESSED = NO

CFLAGS = -O3 -Wall -Wextra
CXXFLAGS = -O3 -Wall -Wextra

FONTDIR = $(SRCDIR)/fonts
FONT_SMALL = $(FONTDIR)/drsans-06.fnt
FONT_SMALL_INC = $(FONTDIR)/drsans-06.inc
FONT_LARGE = $(FONTDIR)/drsans-09.fnt
FONT_LARGE_INC = $(FONTDIR)/drsans-09.inc

DEPS = $(FONT_SMALL_INC) $(FONT_LARGE_INC)

# ----------------------------

include $(shell cedev-config --makefile)

$(FONT_SMALL_INC): $(FONT_SMALL)
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -l 7  -x 3 -c 1 -a 1 -b 1 -w normal -s 0 -s sans-serif -s upright -s proportional $@

$(FONT_LARGE_INC): $(FONT_LARGE)
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -l 7  -x 3 -c 1 -a 1 -b 1 -w normal -s 0 -s sans-serif -s upright -s proportional $@