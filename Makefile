# ----------------------------
# Makefile Options
# ----------------------------

NAME = TiCeG
ICON = icon.png
DESCRIPTION = "A Trading Card Game (TCG) meant for the TI 84 Plus CE family"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

FONTDIR = $(SRCDIR)/fonts
FONT = $(FONTDIR)/drsans.fnt
FONT_INC = $(FONTDIR)/drsans.inc

DEPS = $(FONT_INC)

# ----------------------------

include $(shell cedev-config --makefile)

$(FONT_INC): $(FONT)
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -l 7  -x 3 -c 1 -a 1 -b 1 -w normal -s 0 -s sans-serif -s upright -s proportional $@