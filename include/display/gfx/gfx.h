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
void hkDrawFastChar(u8 x, u8 y, u8 c);
void hkDrawChar(u8 x, u8 y, u8 c, u8 color, u8 size);
void hkDrawFastString(u8 x, u8 y, const char* str);
void hkDrawString(u8 x, u8 y, const char* str, u8 color, u8 size);

static void hkDrawTestPattern() {
    HINFO("Drawing GFX test pattern...");
    hkClearBuffer();

    // 1. Bounding Box - tests drawing at the screen edges
    // We use 127 because coordinates are 0-127.
    hkDrawRect(0, 0, 127, 127, 0x0F);
    sleep_ms(250); // Small delay to see drawing happen in stages
    hkDisplay();

    // 2. Diagonal Lines - test for the hkDrawLine algorithm
    hkDrawLine(0, 0, 127, 127, 0x0F);
    hkDrawLine(0, 127, 127, 0, 0x0F);
    sleep_ms(250);
    hkDisplay();

    // 3. Text - tests both hkDrawChar and hkDrawString
    hkDrawFastString(20, 5, "SSD1327 gfx OK!");
    hkDrawFastString(30, 15, "0123456789");

    // Test some individual characters
    hkDrawFastChar(40, 30, '#');
    hkDrawFastChar(50, 30, '$');
    hkDrawFastChar(60, 30, '%');
    hkDrawFastChar(70, 30, '&');
    hkDrawFastChar(80, 30, '*');
    sleep_ms(250);
    hkDisplay();

    // 4. Shapes - tests rectangles, fill, and alignment
    hkFillRect(8, 45, 50, 25, 0x0F);   // Draw a solid rectangle
    hkDrawRect(6, 43, 54, 29, 0x0F);   // Draw an outline around it
    sleep_ms(250);
    hkDisplay();

    // 5. Fast Lines - tests the optimized horizontal and vertical line functions
    hkDrawFastVLine(100, 45, 30, 0x0F);
    hkDrawFastHLine(80, 80, 40, 0x0F);
    sleep_ms(250);
    hkDisplay();

    // 6. Font scalling and grayscale
    hkDrawChar(32, 117, 'A', 0x01, 1);
    hkDrawChar(40, 110, 'B', 0x06, 2);
    hkDrawChar(53, 103, 'C', 0x0F, 3);
    sleep_ms(250);
    hkDisplay();

    sleep_ms(5000);
    hkClearBuffer();
    hkDisplay();
}