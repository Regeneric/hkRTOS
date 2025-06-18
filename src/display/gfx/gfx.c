#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/rand.h>

#include <core/map.h>
#include <display/display.h>
#include <display/gfx/gfx.h>

void hkClearBuffer(void) {
    HTRACE("gfx.c -> hkClearBuffer(void):void");
    GFX_ClearBuffer();
}
void hkDisplay(void) {
    HTRACE("gfx.c -> hkDisplay(void):void");
    GFX_Display();
}
void hkDrawPixel(u8 x, u8 y, u8 color) {
    HTRACE("gfx.c -> hkDrawPixel(u8, u8, u8):void");
    GFX_DrawPixel(x, y, color);
}
void hkDrawLine(u8 x0, u8 y0, u8 x1, u8 y1, u8 color) {
    HTRACE("gfx.c -> hkDrawLine(u8, u8, u8, u8, u8):void");
    GFX_DrawLine(x0, y0, x1, y1, color);
}
void hkDrawFastHLine(u8 x, u8 y, u8 w, u8 color) {
    HTRACE("gfx.c -> hkDrawFastHLine(u8, u8, u8, u8):void");
    GFX_DrawFastHLine(x, y, w, color);
}
void hkDrawFastVLine(u8 x, u8 y, u8 h, u8 color) {
    HTRACE("gfx.c -> hkDrawFastVLine(u8, u8, u8, u8):void");
    GFX_DrawFastVLine(x, y, h, color);
}
void hkDrawRect(u8 x, u8 y, u8 w, u8 h, u8 color) {
    HTRACE("gfx.c -> hkDrawRect(u8, u8, u8, u8, u8):void");
    GFX_DrawRect(x, y, w, h, color);
}
void hkFillRect(u8 x, u8 y, u8 w, u8 h, u8 color) {
    HTRACE("gfx.c -> hkFillRect(u8, u8, u8, u8, 8):void");
    GFX_FillRect(x, y, w, h, color);
}
void hkFillFastRect(u8 x, u8 y, u8 w, u8 h, u8 color) {
    HTRACE("gfx.c -> hkFillFastRect(u8, u8, u8, u8, 8):void");
    GFX_FillFastRect(x, y, w, h, color);
}
void hkDrawFastChar(u8 x, u8 y, u8 c) {
    HTRACE("gfx.c -> hkDrawFastChar(u8, u8, u8):void");
    GFX_DrawChar(x, y, c, 0x0F, 1);
}
void hkDrawChar(u8 x, u8 y, u8 c, u8 color, u8 size) {
    HTRACE("gfx.c -> hkDrawChar(u8, u8, u8, u8, u8):void");
    GFX_DrawChar(x, y, c, color, size);
}
void hkDrawFastString(u8 x, u8 y, const char* str) {
    HTRACE("gfx.c -> hkDrawFastString(u8, u8, const char*):void");
    GFX_DrawString(x, y, str, 0x0F, 1);
}
void hkDrawString(u8 x, u8 y, const char* str, u8 color, u8 size) {
    HTRACE("gfx.c -> hkDrawString(u8, u8, const char*, u8, u8):void");
    GFX_DrawString(x, y, str, color, size);
}

void hkDrawTestPattern() {
    HTRACE("gfx.c -> hkDrawTestPattern(void):void");
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

void hkGraphInit(GraphConfig_t* config) {
    HTRACE("gfx.c -> hkGraphInit(GraphConfig_t*):void");
    memset(config->history, 0, config->length);
}
void hkGraphDrawAxes(const GraphConfig_t* config) {
    HTRACE("gfx.c -> hkGraphDrawAxes(const GraphConfig_t*):void");
    hkDrawRect(config->x-1, config->y-1, config->width+2, config->height+2, config->borderColour);
}

void hkGraphAddDataPoint(GraphConfig_t* config, f32 data) {
    HTRACE("gfx.c -> hkGraphAddDataPoint(GraphConfig_t*, f32):void");

    i32 nextY = map((i32)(data * 10), (i32)(config->minVal * 10), (i32)(config->maxVal * 10), (i32)(config->y + config->height), (i32)config->y);

    if(nextY < config->y) nextY = config->y;
    if(nextY > config->y + config->height) nextY = (config->y + config->height);

    config->history[config->cursorX] = nextY;
    config->cursorX++;

    if(config->cursorX >= config->width) {config->cursorX = 0;}
}

void hkGraphDraw(const GraphConfig_t* config) {
    HTRACE("gfx.c -> hkGraphDraw(const GraphConfig_t*, f32):void");

    for(u16 i = 1; i < config->cursorX; i++) {
        i32 prevY = config->history[i-1];
        i32 nextY = config->history[i];
        
        if(prevY < config->y) prevY = config->y;
        if(prevY > config->y + config->height) prevY = config->y + config->height;
        
        if(nextY < config->y) nextY = config->y;
        if(nextY > config->y + config->height) nextY = config->y + config->height;

        hkDrawLine((config->x + i - 1), prevY, (config->x + i), nextY, config->colour);
    }
}

void hkGraphDrawLegend(const GraphConfig_t* config, const char* title) {
    HTRACE("gfx.c -> hkGraphDrawLegend(const GraphConfig_t*, const char*):void");

    char label[16];
    u8 titleX = config->x + (config->width/2) - (strlen(title) * 6 / 2);
    hkDrawString(titleX, (config->y - 10), title, config->borderColour, 1);

    snprintf(label, sizeof(label), "%.0f", config->maxVal);
    hkDrawString(6, (config->y + 1), label, config->legendColour, 1);

    snprintf(label, sizeof(label), "%.0f", config->minVal);
    hkDrawString(6, (config->y + config->height - 8), label, config->legendColour, 1);
}

static u8 GFX_PrecisionPrint(u8 precision, f32 data, char* buffer, size_t len) {
    HTRACE("gfx.c -> s:GFX_PrecisionPrint(u8, f32, char*, size_t):u8");

    switch(precision) {
        case 0:
            return snprintf(buffer, len, "%.0f", data);
            break;
        case 1:
            return snprintf(buffer, len, "%.1f", data);
            break;
        case 2:
            return snprintf(buffer, len, "%.2f", data);
            break;
        case 3:
            return snprintf(buffer, len, "%.3f", data);
            break;
        default:
            return snprintf(buffer, len, "%.1f", data);
            break;
    }
}

void hkGraphDrawValue(const GraphConfig_t* config, const f32 data, const u8 precision) {
    HTRACE("gfx.c -> hkGraphDrawValue(const GraphConfig_t*, const f32, const u8):void");

    char label[16];
    u8 padding = GFX_PrecisionPrint(precision, data, label, sizeof(label));

    u8 labelX = config->x + config->width - (padding*6);
    hkDrawString(labelX, (config->y + 1), label, config->legendColour, 1);
}

void hkGraphDrawValueFollow(const GraphConfig_t* config, const f32 data, const u8 precision) {
    HTRACE("gfx.c -> hkGraphDrawValueFollow(const GraphConfig_t*, const f32, const u8):void");

    char label[16];
    u8 padding = GFX_PrecisionPrint(precision, data, label, sizeof(label));

    u16 idx = (config->cursorX == 0) ? (config->width - 1) : (config->cursorX - 1);
    i16 currY = config->history[idx];

    const u8 fontHeight = 7;    // TODO: it shouldn't be hardcoded 
    const u8 topPadding = 4;    // TODO: it shouldn't be hardcoded 

    u8 graphTopY    = config->y;
    u8 graphBottomY = config->y +  config->height;
    u8 graphMidY    = config->y + (config->height/2);

    u8 drawY = 0;

    if(currY < graphMidY) {
        drawY = currY + topPadding;
        if(drawY < graphTopY) drawY = graphTopY;
    } else {
        drawY = currY - fontHeight - topPadding;
        if((drawY + fontHeight) > graphBottomY) drawY = graphBottomY - fontHeight;
    }

    u8 textWidth = (padding*5)+padding; // Num of characters * font width + num of characters for spacing; TODO: font widht shouldn't be hardcoded 
    u8 textHeight = 7;                  // TODO: it shouldn't be hardcoded 
    u16 currX = config->x + idx - (textWidth/2);

    if(currX < config->x)  currX = config->x;
    if(currX + textWidth > config->x + config->width) currX = config->x + config->width - textWidth;

    hkDrawString(currX, drawY, label, config->legendColour, 1);
}



// Fun stuff
typedef struct Raindrop_t {
    u8  x, y;
    f32 speedY;
    u8  width;
    u8  height;
    u8  colour;
} Raindrop_t;
static Raindrop_t droplets[48];

void hkScreenSaverInit() {
    for(u8 i = 0; i < 48; ++i) {
        droplets[i].x = get_rand_32() % 127;
        droplets[i].y = get_rand_32() % 10 - 10;
        droplets[i].speedY = get_rand_32() % 15 + 6;
        droplets[i].width  = get_rand_32() % 3;
        droplets[i].height = get_rand_32() % 4 + 7;
        droplets[i].colour = get_rand_32() % 15 + 3;
    }
}

void hkScreenSaver(u8 width, u8 height, u8 screenSaver) {
    HTRACE("gfx.c -> hkScreenSaver(u8, u8):void");

    hkClearBuffer();

    switch(screenSaver) {
        case 0:
            for(u8 i = 0; i < 48; ++i) {
                droplets[i].y += droplets[i].speedY;
                droplets[i].speedY += 0.18;

                hkFillFastRect(droplets[i].x, droplets[i].y, droplets[i].width, droplets[i].height, droplets[i].colour);

                if(droplets[i].y > 127) {
                    droplets[i].x = get_rand_32() % 120 + 7;
                    droplets[i].y = get_rand_32() % 10 - 10;
                    droplets[i].width  = get_rand_32() % 3;
                    droplets[i].speedY = get_rand_32() % 15 + 6;
                }
            }
            break;
        case 1:
            static u8 rectX = 10, rectY = 10;
            static u8 speedX = 1, speedY = 1;

            rectX += speedX;
            rectY += speedY;

            if(rectX <= 0 || rectX >= (width  - 10)) speedX *= -1;
            if(rectY <= 0 || rectY >= (height - 10)) speedY *= -1;

            hkFillFastRect(rectX, rectY, 10, 10, 15);
            break;
        default:
            screenSaver = 0;
            break;

    }

    hkDisplay();
}