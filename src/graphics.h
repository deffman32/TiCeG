#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <graphx.h>
#include <math.h>

#define CONVERT_ANGLE(deg) (roundf(deg * (255.0 / 360.0)))

__attribute__((always_inline)) float atan2_approximation1(float y, float x) {
  // http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
  // Volkan SALMA

  const float ONEQTR_PI = M_PI / 4.0;
  const float THRQTR_PI = 3.0 * M_PI / 4.0;
  float r, angle;
  float abs_y = fabs(y) + 1e-10f; // kludge to prevent 0/0 condition
  if (x < 0.0f) {
    r = (x + abs_y) / (abs_y - x);
    angle = THRQTR_PI;
  } else {
    r = (x - abs_y) / (x + abs_y);
    angle = ONEQTR_PI;
  }
  angle += (0.1963f * r * r - 0.9817f) * r;
  if (y < 0.0f)
    return (-angle); // negate if in quad III or IV
  else
    return (angle);
}

__attribute__((always_inline)) static inline void
gfx_Arc(const int center_x, const int center_y, const int inner_radius,
        const int outer_radius, const int angle_start, const int angle_end,
        const bool approx_trig) {

  int x, y, d, r, ratio;

  const bool inverted = angle_start > angle_end;
  const bool full = angle_start == angle_end;
  const int a_start = inverted ? angle_end : angle_start;
  const int a_end = inverted ? angle_start : angle_end;

  for (r = inner_radius; r <= outer_radius; r++) {
    x = 0;
    y = r;
    d = r - 1;

    while (y >= x) {
      if (approx_trig) {
        ratio = x * 255 / y; // x/y [0..255]
        ratio = ratio * (770195 - (ratio - 255) * (ratio + 941)) /
                6137491; // arctan(x/y) [0..32]
      } else {
        ratio = floorf((M_PI_2 - atan2_approximation1(y, x)) * 32 / M_PI_4);
      }

      if (full || ((ratio >= a_start && ratio < a_end) ^ inverted)) {
        gfx_SetPixel(center_x + y, center_y - x);
      }
      if (full ||
          ((ratio > (63 - a_end) && ratio <= (63 - a_start)) ^ inverted)) {
        gfx_SetPixel(center_x + x, center_y - y);
      }
      if (full ||
          ((ratio >= (a_start - 64) && ratio < (a_end - 64)) ^ inverted)) {
        gfx_SetPixel(center_x - x, center_y - y);
      }
      if (full ||
          ((ratio > (127 - a_end) && ratio <= (127 - a_start)) ^ inverted)) {
        gfx_SetPixel(center_x - y, center_y - x);
      }
      if (full ||
          ((ratio >= (a_start - 128) && ratio < (a_end - 128)) ^ inverted)) {
        gfx_SetPixel(center_x - y, center_y + x);
      }
      if (full ||
          ((ratio > (191 - a_end) && ratio <= (191 - a_start)) ^ inverted)) {
        gfx_SetPixel(center_x - x, center_y + y);
      }
      if (full ||
          ((ratio >= (a_start - 192) && ratio < (a_end - 192)) ^ inverted)) {
        gfx_SetPixel(center_x + x, center_y + y);
      }
      if (full ||
          ((ratio > (255 - a_end) && ratio <= (255 - a_start)) ^ inverted)) {
        gfx_SetPixel(center_x + y, center_y + x);
      }

      if (d >= 2 * x) {
        d = d - 2 * x - 1;
        x = x + 1;
      } else if (d < 2 * (r - y)) {
        d = d + 2 * y - 1;
        y = y - 1;
      } else {
        d = d + 2 * (y - x - 1);
        y = y - 1;
        x = x + 1;
      }
    }
  }
}

void gfx_RoundedRect(int x1, int y1, int x2, int y2, int r) {
  int f = 1 - r;
  int ddF_x = 0;
  int ddF_y = -2 * r;
  int x = 0;
  int y = r;
  gfx_Line(x1 + r, y1, x2 - r, y1);
  gfx_Line(x1 + r, y2, x2 - r, y2);
  gfx_Line(x1, y1 + r, x1, y2 - r);
  gfx_Line(x2, y1 + r, x2, y2 - r);
  int cx1 = x1 + r;
  int cx2 = x2 - r;
  int cy1 = y1 + r;
  int cy2 = y2 - r;
  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x + 1;
    gfx_SetPixel(cx2 + x, cy2 + y);
    gfx_SetPixel(cx1 - x, cy2 + y);
    gfx_SetPixel(cx2 + x, cy1 - y);
    gfx_SetPixel(cx1 - x, cy1 - y);
    gfx_SetPixel(cx2 + y, cy2 + x);
    gfx_SetPixel(cx1 - y, cy2 + x);
    gfx_SetPixel(cx2 + y, cy1 - x);
    gfx_SetPixel(cx1 - y, cy1 - x);
    gfx_SetPixel(cx2 + x, cy2 + y);
    gfx_SetPixel(cx1 - x, cy2 + y);
    gfx_SetPixel(cx2 + x, cy1 - y);
    gfx_SetPixel(cx1 - x, cy1 - y);
    gfx_SetPixel(cx2 + y, cy2 + x);
    gfx_SetPixel(cx1 - y, cy2 + x);
    gfx_SetPixel(cx2 + y, cy1 - x);
    gfx_SetPixel(cx1 - y, cy1 - x);
  }
}

__attribute__((always_inline)) inline void
gfx_FillRoundedRect(int x1, int y1, int x2, int y2, int radius) {
  x1 = min(x1, x2);
  x2 = max(x1, x2);
  y1 = min(y1, y2);
  y2 = max(y1, y2);
  int r = min(radius, min(x2 - x1, y2 - y1));
  int height = y2 - y1;
  int width = x2 - x1;
  gfx_FillRectangle(x1, y1 + r, width + 1, height - r * 2);
  gfx_FillRectangle(x1 + r, y1, width - r * 2, height + 1);
  gfx_FillCircle(x1 + r, y1 + r, r);
  gfx_FillCircle(x2 - r, y1 + r, r);
  gfx_FillCircle(x1 + r, y2 - r, r);
  gfx_FillCircle(x2 - r, y2 - r, r);
}

#endif