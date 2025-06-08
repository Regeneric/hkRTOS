#pragma once
#include <defines.h>

void hkClearBuffer(void);
void hkDisplay(void);
void hkDrawPixel(u8 x, u8 y, u8 color);
void hkDrawLine(u8 x0, u8 y0, u8 x1, u8 y1, u8 color);
void hkDrawFastHLine(u8 x, u8 y, u8 w, u8 color);
void hkDrawFastVLine(u8 x, u8 y, u8 h, u8 color);
void hkDrawRect(u8 x, u8 y, u8 w, u8 h, u8 color);
void hkFillRect(u8 x, u8 y, u8 w, u8 h, u8 color);
void hkDrawChar(u8 x, u8 y, u8 c, u8 color);
void hkDrawString(u8 x, u8 y, const char* str, u8 color);