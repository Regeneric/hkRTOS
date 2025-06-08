#include <config/arm.h>
#if hkOLED_SH1107

#include <string.h>
#include <display/display.h>
#include <display/gfx/font_5_7.h>
#include <display/sh1107/sh1107_config.h>

typedef struct GFX_InternalState_t {
    u8 frameBuffer[(SH1107_WIDTH*SH1107_HEIGHT)/8];
    u8 cursorX;
    u8 cursorY;
    u8 textSize;
} GFX_InternalState_t;
static GFX_InternalState_t sgGFX_InternalState;

void GFX_ClearBuffer() {
    HTRACE("sh1107_gfx.c -> GFX_ClearBuffer(void):void");
    memset(sgGFX_InternalState.frameBuffer, 0, sizeof(sgGFX_InternalState.frameBuffer));
    sgGFX_InternalState.cursorX = 0;
    sgGFX_InternalState.cursorY = 0;
}

void GFX_Display() {
    HTRACE("sh1107_gfx.c -> GFX_Display(void):void");

     // Set the memory "window" to the entire display for a full refresh
    static const u8 pageAddress[] = {SH1107_PAGEADDR, 0, 15};       // Pages 0-15 for 128 rows
    Display_WriteCommandList(pageAddress, sizeof(pageAddress));

    static const u8 columnAddress[] = {SH1107_COLUMNADDR, 0, 127};  //  Columns 0-127
    Display_WriteCommandList(columnAddress, sizeof(columnAddress));

    // Stream the whole framebuffer
    Display_WriteData(sgGFX_InternalState.frameBuffer, sizeof(sgGFX_InternalState.frameBuffer));
}

void GFX_DrawPixel(u8 x, u8 y, u8 color) {
    HTRACE("sh1107_gfx.c -> GFX_DrawPixel(u8, u8, u8):void");
    
    if(x >= SH1107_WIDTH || y >= SH1107_HEIGHT) {
        HDEBUG("GFX_DrawPixel(): Passed X or Y value is outside the scope!");
        return;
    }

    u16 byteIndex = x + (y/8) * SH1107_WIDTH;
    u8  bitMask   = 1 << (y%8);

    if(color) sgGFX_InternalState.frameBuffer[byteIndex] |= bitMask;    // Set pixel in the framebuffer
    else sgGFX_InternalState.frameBuffer[byteIndex] &= ~bitMask;        // Clear pixel in the framebuffer
}

void GFX_DrawLine(u8 x0, u8 y0, u8 x1, u8 y1, u8 color) {
    HTRACE("sh1107_gfx.c -> GFX_DrawLine(u8, u8, u8, u8, u8):void");

    // Bresenham's line algorithm
    i16 dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    i16 dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    i16 sx = (x0 < x1) ?  1 : -1;
    i16 sy = (y0 < y1) ?  1 : -1;
    i16 err = dx - dy;

    while(FOREVER) {
        GFX_DrawPixel(x0, y0, color);
        if(x0 == x1 && y0 == y1) break;
        
        i16 e = err*2;
        if(e > -dy) {
            err -= dy;
            x0 += sx;
        }

        if(e < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void GFX_DrawFastHLine(u8 x, u8 y, u8 w, u8 color) {
    HTRACE("sh1107_gfx.c -> GFX_DrawFastHLine(u8, u8, u8, u8):void");
    for(u8 i = 0; i < w; ++i) GFX_DrawPixel(x+i, y, color);
}

void GFX_DrawFastVLine(u8 x, u8 y, u8 h, u8 color) {
    HTRACE("sh1107_gfx.c -> GFX_DrawFastVLine(u8, u8, u8, u8):void");
    for(u8 i = 0; i < h; ++i) GFX_DrawPixel(x, y+i, color);
}

void GFX_DrawRect(u8 x, u8 y, u8 w, u8 h, u8 color) {
    HTRACE("sh1107_gfx.c -> GFX_DrawRect(u8, u8, u8, u8):void");

    GFX_DrawFastHLine(x, y, w, color);      // Top edge
    GFX_DrawFastHLine(x, y+h-1, w, color);  // Bottom edge
    GFX_DrawFastVLine(x, y, h, color);      // Left edge
    GFX_DrawFastVLine(x+w-1, y, h, color);  // Right edge
}

void GFX_FillRect(u8 x, u8 y, u8 w, u8 h, u8 color) {
    HTRACE("sh1107_gfx.c -> GFX_FillRect(u8, u8, u8, u8):void");
    for(u8 i = 0; i < h; ++i) GFX_DrawFastHLine(x, y+i, w, color);
}

void GFX_DrawChar(u8 x, u8 y, u8 c, u8 color) {
    HTRACE("sh1107_gfx.c -> GFX_DrawChar(u8, u8, u8, u8):void");

    if(c < 32 || c > 126) {
        HDEBUG("GFX_DrawChar(): Could not find character %c in the font array", c);
        return;
    }

    u16 fontIndex = (c-32) * 5;
    for(u8 col = 0; col < 5; col++) {
        u8 line = FONT_5x7[fontIndex + col];
        for(u8 row = 0; row < 8; row++) {
            if((line >> row) & 0x1) GFX_DrawPixel(x+col, y+row, color);
        }
    }
}

void GFX_DrawString(u8 x, u8 y, const char* str, u8 color) {
    sgGFX_InternalState.cursorX = x;
    sgGFX_InternalState.cursorY = y;

    while(*str) {
        GFX_DrawChar(sgGFX_InternalState.cursorX, sgGFX_InternalState.cursorY, *str, color);
        sgGFX_InternalState.cursorX += (5 * sgGFX_InternalState.textSize) + 1;
        str++;
    }
}

#endif