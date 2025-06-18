#include <config/arm.h>
#if hkOLED_SSD1327

#include <string.h>

#include <display/display.h>
#include <display/gfx/font_5_7.h>
#include <display/ssd1327/ssd1327_config.h>

typedef struct GFX_InternalState_t {
    u8 frameBuffer[(SSD1327_WIDTH * SSD1327_HEIGHT) / 2];
    u8 cursorX;
    u8 cursorY;
    u8 textSize;
} GFX_InternalState_t;
static GFX_InternalState_t sgGFX_InternalState;

void GFX_ClearBuffer() {
    HTRACE("ssd1327_gfx.c -> GFX_ClearBuffer(void):void");
    memset(sgGFX_InternalState.frameBuffer, 0, sizeof(sgGFX_InternalState.frameBuffer));
    sgGFX_InternalState.cursorX = 0;
    sgGFX_InternalState.cursorY = 0;
    sgGFX_InternalState.textSize = 1;   // TODO: for now it's hardcoded
}

void GFX_Display() {
    HTRACE("ssd1327_gfx.c -> GFX_Display(void):void");

    // Set Column Address Range. For SSD1327, this is in 4-bit segments.
    // So 128 pixels wide is 128/2 = 64 bytes. The range is 0 to 63.
    static const u8 columnAddress[] = {SSD1327_SET_COL_ADDR, 0x00, 0x3F}; // 0 to 63
    Display_WriteCommandList(columnAddress, sizeof(columnAddress));

    // Set Row Address Range. This is a direct pixel row.
    static const u8 rowAddress[] = {SSD1327_SET_ROW_ADDR, 0x00, 0x7F};   // 0 to 127
    Display_WriteCommandList(rowAddress, sizeof(rowAddress));

    // Stream the whole framebuffer
    Display_WriteData(sgGFX_InternalState.frameBuffer, sizeof(sgGFX_InternalState.frameBuffer));
}

void GFX_DrawPixel(u8 x, u8 y, u8 color) {
    HTRACE("ssd1327_gfx.c -> GFX_DrawPixel(u8, u8, u8):void");
    
    if(x >= SSD1327_WIDTH || y >= SSD1327_HEIGHT) {
        HTRACE("GFX_DrawPixel(): Passed X or Y value is outside the scope!");
        return;
    }

    u8 grayscaleColor = color ? 0x0F : 0x00;
    u16 index = (y * SSD1327_WIDTH + x) / 2;

    if(x % 2 == 0) {
        // The pixel is in an EVEN column (0, 2, 4...). It's stored in the HIGH nibble.
        // First, clear the high nibble (AND with 0b00001111)
        sgGFX_InternalState.frameBuffer[index] &= 0x0F;
        // Then, set the high nibble with our color (OR with color shifted left by 4)
        sgGFX_InternalState.frameBuffer[index] |= (color << 4);
    } else {
        // The pixel is in an ODD column (1, 3, 5...). It's stored in the LOW nibble.
        // First, clear the low nibble (AND with 0b11110000)
        sgGFX_InternalState.frameBuffer[index] &= 0xF0;
        // Then, set the low nibble with our color
        sgGFX_InternalState.frameBuffer[index] |= color;
    }
}

void GFX_DrawLine(u8 x0, u8 y0, u8 x1, u8 y1, u8 color) {
    HTRACE("ssd1327_gfx.c -> GFX_DrawLine(u8, u8, u8, u8, u8):void");

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
    HTRACE("ssd1327_gfx.c -> GFX_DrawFastHLine(u8, u8, u8, u8):void");
    for(u8 i = 0; i < w; ++i) GFX_DrawPixel(x+i, y, color);
}

void GFX_DrawFastVLine(u8 x, u8 y, u8 h, u8 color) {
    HTRACE("ssd1327_gfx.c -> GFX_DrawFastVLine(u8, u8, u8, u8):void");
    for(u8 i = 0; i < h; ++i) GFX_DrawPixel(x, y+i, color);
}

void GFX_DrawRect(u8 x, u8 y, u8 w, u8 h, u8 color) {
    HTRACE("ssd1327_gfx.c -> GFX_DrawRect(u8, u8, u8, u8, u8):void");

    GFX_DrawFastHLine(x, y, w, color);      // Top edge
    GFX_DrawFastHLine(x, y+h-1, w, color);  // Bottom edge
    GFX_DrawFastVLine(x, y, h, color);      // Left edge
    GFX_DrawFastVLine(x+w-1, y, h, color);  // Right edge
}

void GFX_FillRect(u8 x, u8 y, u8 w, u8 h, u8 color) {
    HTRACE("ssd1327_gfx.c -> GFX_FillRect(u8, u8, u8, u8, u8):void");
    for(u8 i = 0; i < h; ++i) GFX_DrawFastHLine(x, y+i, w, color);
}

void GFX_FillFastRect(u8 x, u8 y, u8 w, u8 h, u8 color) {
    HTRACE("ssd1327_gfx.c -> GFX_FillFastRect(u8, u8, u8, u8, u8):void");

    if (x >= SSD1327_WIDTH || y >= SSD1327_HEIGHT) return;
    if (x + w < 0 || y + h < 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SSD1327_WIDTH)  w = SSD1327_WIDTH  - x;
    if (y + h > SSD1327_HEIGHT) h = SSD1327_HEIGHT - y;

    for(i16 j = 0; j < h; j++) {
        for(i16 i = 0; i < w; i++) {
            // This is just an example of GFX_DrawPixel's logic.
            // Replace this with the actual logic from your GFX_DrawPixel function
            // that writes to your screen_buffer.
            int16_t px = x + i;
            int16_t py = y + j;

            u16 index = (py * SSD1327_WIDTH + px) / 2;
            if(x % 2 == 0) {
                // The pixel is in an EVEN column (0, 2, 4...). It's stored in the HIGH nibble.
                // First, clear the high nibble (AND with 0b00001111)
                sgGFX_InternalState.frameBuffer[index] &= 0x0F;
                // Then, set the high nibble with our color (OR with color shifted left by 4)
                sgGFX_InternalState.frameBuffer[index] |= (color << 4);
            } else {
                // The pixel is in an ODD column (1, 3, 5...). It's stored in the LOW nibble.
                // First, clear the low nibble (AND with 0b11110000)
                sgGFX_InternalState.frameBuffer[index] &= 0xF0;
                // Then, set the low nibble with our color
                sgGFX_InternalState.frameBuffer[index] |= color;
            }
        }
    }
}

void GFX_DrawFastChar(u8 x, u8 y, u8 c) {
    HTRACE("ssd1327_gfx.c -> GFX_DrawFastChar(u8, u8, u8):void");
    GFX_DrawChar(x, y, c, 0x0F, 1);
}
void GFX_DrawChar(u8 x, u8 y, u8 c, u8 color, u8 size) {
    HTRACE("ssd1327_gfx.c -> GFX_DrawChar(u8, u8, u8, u8, u8):void");

    sgGFX_InternalState.textSize = (size > 1) ? size : 1;
    if(c < 32 || c > 126) {
        HWARN("GFX_DrawChar(): Could not find character %c in the font array", c);
        return;
    }

    u16 fontIndex = (c-32) * 5;
    for(u8 col = 0; col < 5; col++) {
        u8 line = FONT_5x7[fontIndex + col];
        for(u8 row = 0; row < 7; row++) {
            if((line >> row) & 0x1) {
                if(size == 1) GFX_DrawPixel(x+col, y+row, color);
                else GFX_FillRect(x + (col * size), y + (row * size), size, size, color);
            }
        }
    }
}

void GFX_DrawFastString(u8 x, u8 y, const char* str) {
    HTRACE("ssd1327_gfx.c -> GFX_DrawFastString(u8, u8, const char*):void");
    GFX_DrawString(x, y, str, 0x0F, 1);
}
void GFX_DrawString(u8 x, u8 y, const char* str, u8 color, u8 size) {
    HTRACE("ssd1327_gfx.c -> GFX_DrawString(u8, u8, u8, const char*, u8, u8):void");
    sgGFX_InternalState.cursorX = x;
    sgGFX_InternalState.cursorY = y;
    sgGFX_InternalState.textSize = size;

    while(*str) {
        GFX_DrawChar(sgGFX_InternalState.cursorX, sgGFX_InternalState.cursorY, *str, color, size);
        sgGFX_InternalState.cursorX += (5 * size) + 1;
        str++;
    }
}

#endif