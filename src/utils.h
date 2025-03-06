#ifndef UTILS_H
#define UTILS_H
#include <fontlibc.h>
#include <stdint.h>
#include <string.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

template <typename T> T min(T a, T b) {
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

template <typename T> T max(T a, T b) {
  if (a > b) {
    return a;
  } else {
    return b;
  }
}

inline void limit_length(char *str, const size_t max_len) {
  if (strlen(str) > max_len) {
    str[max_len] = '\0';
  }
}

#define fontlib_DrawStringXY(string, x, y)                                     \
  fontlib_SetCursorPosition(x, y);                                             \
  fontlib_DrawString(string)

// Store metadata like ptr length
template <typename T> struct fat_ptr {
  T *ptr;
  size_t size;
};

template <typename T> constexpr fat_ptr<T> fat_nullptr = {nullptr, 0};

template <typename T> constexpr inline bool is_fat_nullptr(fat_ptr<T> fp) {
  return fp.ptr == nullptr || fp.size == 0;
}

static const char *const digit_table[100] = {
    "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",  "10", "11",
    "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23",
    "24", "25", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35",
    "36", "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "70", "71",
    "72", "73", "74", "75", "76", "77", "78", "79", "80", "81", "82", "83",
    "84", "85", "86", "87", "88", "89", "90", "91", "92", "93", "94", "95",
    "96", "97", "98", "99"};

void ucta(unsigned char value, char *str) {
  if (value < 100) {
    strcpy(str, digit_table[value]);
    return;
  }

  if (value < 200) {
    str[0] = '1';
    strcpy(str + 1, digit_table[value - 100]);
    return;
  }

  str[0] = '2';
  strcpy(str + 1, digit_table[value - 200]);
}

#endif