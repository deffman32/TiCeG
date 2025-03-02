#ifndef UTILS_H
#define UTILS_H
#include <fontlibc.h>
#include <stdint.h>
#include <string.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

inline void limit_length(char *str, const size_t max_len) {
  if (strlen(str) > max_len) {
    str[max_len] = '\0';
  }
}

static uint24_t fontlib_DrawStringXY(const char *string, const unsigned int x,
                                     const uint8_t y) {
  fontlib_SetCursorPosition(x, y);
  return fontlib_DrawString(string);
}

// Store metadata like ptr length
template <typename T> struct fat_ptr {
  T *ptr;
  size_t size;
};

#endif