#include <display/display.h>
#include <display/gfx/gfx.h>

void hkClearBuffer(void) {GFX_ClearBuffer();}
void hkDisplay(void) {GFX_Display();}
void hkDrawPixel(u8 x, u8 y, u8 color) {GFX_DrawPixel(x, y, color);}
void hkDrawLine(u8 x0, u8 y0, u8 x1, u8 y1, u8 color) {GFX_DrawLine(x0, y0, x1, y1, color);}
void hkDrawFastHLine(u8 x, u8 y, u8 w, u8 color) {GFX_DrawFastHLine(x, y, w, color);}
void hkDrawFastVLine(u8 x, u8 y, u8 h, u8 color) {GFX_DrawFastVLine(x, y, h, color);}
void hkDrawRect(u8 x, u8 y, u8 w, u8 h, u8 color) {GFX_DrawRect(x, y, w, h, color);}
void hkFillRect(u8 x, u8 y, u8 w, u8 h, u8 color) {GFX_FillRect(x, y, w, h, color);}
void hkDrawChar(u8 x, u8 y, u8 c, u8 color) {GFX_DrawChar(x, y, c, color);}
void hkDrawString(u8 x, u8 y, const char* str, u8 color) {GFX_DrawString(x, y, str, color);}