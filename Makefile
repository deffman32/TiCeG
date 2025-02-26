# ----------------------------
# Makefile Options
# ----------------------------

NAME = TiCeG
ICON = icon.png
DESCRIPTION = "A Trading Card Game (TCG) meant for the TI 84 Plus CE family"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
