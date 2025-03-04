# ----------------------------
# Makefile Options
# ----------------------------

NAME = TiCeG
ICON = icon.png
DESCRIPTION = "A Trading Card Game (TCG) meant for the TI 84 Plus CE family"
COMPRESSED = YES
COMPRESSED_MODE = zx0

CFLAGS = -O3 -Wall -Wextra
CXXFLAGS = -O3 -Wall -Wextra

FONTDIR = $(SRCDIR)/fonts

DEPS = $(FONTDIR)/cherry-10.inc $(FONTDIR)/cherry-13.inc $(FONTDIR)/cherry-20.inc $(FONTDIR)/cherry-26.inc

# ----------------------------

include $(shell cedev-config --makefile)

$(FONTDIR)/cherry-10.inc: $(FONTDIR)/cherry-10.fnt
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -a 1 -b 1 -w bold -Z $@
$(FONTDIR)/cherry-13.inc: $(FONTDIR)/cherry-13.fnt
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -a 1 -b 1 -w bold -Z $@
$(FONTDIR)/cherry-20.inc: $(FONTDIR)/cherry-20.fnt
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -a 1 -b 1 -w bold -Z $@
$(FONTDIR)/cherry-26.inc: $(FONTDIR)/cherry-26.fnt
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -a 1 -b 1 -w bold -Z $@