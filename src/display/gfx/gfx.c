#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <core/map.h>

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
void hkDrawFastChar(u8 x, u8 y, u8 c) {GFX_DrawChar(x, y, c, 0x0F, 1);}
void hkDrawChar(u8 x, u8 y, u8 c, u8 color, u8 size) {GFX_DrawChar(x, y, c, color, size);}
void hkDrawFastString(u8 x, u8 y, const char* str) {GFX_DrawString(x, y, str, 0x0F, 1);}
void hkDrawString(u8 x, u8 y, const char* str, u8 color, u8 size) {GFX_DrawString(x, y, str, color, size);}

void hkDrawTestPattern() {
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

void hkScreenSaver(u8 width, u8 height) {
    hkClearBuffer();

    static u8 rectX = 10;
    static u8 rectY = 10;
    static u8 speedX = 1;
    static u8 speedY = 1;

    rectX += speedX;
    rectY += speedY;

    if(rectX <= 0 || rectX >= (width - 10)) {
        speedX *= -1;
    }

    if(rectY <= 0 || rectY >= (height - 10)) {
        speedY *= -1;
    }

    hkFillRect(rectX, rectY, 10, 10, 15);
    hkDisplay();
}


void hkGraphInit(GraphConfig_t* config) {memset(config->history, 0, config->length);}
void hkGraphDrawAxes(const GraphConfig_t* config) {hkDrawRect(config->x-1, config->y-1, config->width+2, config->height+2, config->borderColour);}
void hkGraphAddDataPoint(GraphConfig_t* config, f32 data) {
    i32 nextY = map((i32)(data * 10), (i32)(config->minVal * 10), (i32)(config->maxVal * 10), (i32)(config->y + config->height), (i32)config->y);

    if(nextY < config->y) nextY = config->y;
    if(nextY > config->y + config->height) nextY = (config->y + config->height);

    config->history[config->cursorX] = nextY;
    config->cursorX++;

    if(config->cursorX >= config->width) {config->cursorX = 0;}
}

void hkGraphDraw(const GraphConfig_t* config) {
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
    char label[16];

    u8 titleX = config->x + (config->width/2) - (strlen(title) * 6 / 2);
    hkDrawString(titleX, (config->y - 10), title, config->borderColour, 1);

    snprintf(label, sizeof(label), "%.0f", config->maxVal);
    hkDrawString(6, (config->y + 1), label, config->legendColour, 1);

    snprintf(label, sizeof(label), "%.0f", config->minVal);
    hkDrawString(6, (config->y + config->height - 8), label, config->legendColour, 1);
}

static u8 GFX_PrecisionPrint(u8 precision, f32 data, char* buffer, size_t len) {
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
    char label[16];
    u8 padding = GFX_PrecisionPrint(precision, data, label, sizeof(label));

    u8 labelX = config->x + config->width - (padding*6);
    hkDrawString(labelX, (config->y + 1), label, config->legendColour, 1);
}

void hkGraphDrawValueFollow(const GraphConfig_t* config, const f32 data, const u8 precision) {
    char label[16];
    u8 padding = GFX_PrecisionPrint(precision, data, label, sizeof(label));

    u16 idx = (config->cursorX == 0) ? (config->width - 1) : (config->cursorX - 1);
    i16 currY = config->history[idx];

    if(currY >= config->height - 7) currY -= 9;
    else currY += 9;

    // Clamp Y axis
    if(currY < config->y) currY = (config->y + 7);
    if(currY > config->y + config->height) currY = (config->y + config->height) - 7;

    // Clamp X axis
    u8 currX = (config->x + idx - ((padding*5)/2));    
    if(currX >= (config->width - config->x))     currX = ((padding*5)/2)-4;
    if(currX >= (config->width - (padding*6)))   currX = (config->width - (padding*6)+4); 
    if(currX <= (((padding*5)/2) + config->x-8)) currX = ((padding*6)/2) - config->x;  // ((padding*5)/2)-4;


    hkDrawString(currX, currY, label, config->legendColour, 1);
}