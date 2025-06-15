#pragma once
#include <defines.h>

#define GRAPH_X_POS     5
#define GRAPH_Y_POS     30
#define GRAPH_WIDTH     118
#define GRAPH_HEIGHT    90

#define MAX_GRAPH_WIDTH 128
typedef struct GraphConfig_t {
    u8  x, y;
    u8  width, height;
    f32 minVal;
    f32 maxVal;
    u8  colour;
    u8  borderColour;
    u8  legendColour;
    u8* history;
    u8  length;
    u8  cursorX;
} GraphConfig_t;

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

void hkDrawTestPattern();
void hkScreenSaver(u8 width, u8 height);

void hkGraphInit(GraphConfig_t* config);
void hkGraphDrawAxes(const GraphConfig_t* config);
void hkGraphAddDataPoint(GraphConfig_t* config, f32 data);
void hkGraphDrawLegend(const GraphConfig_t* config, const char* title);
void hkGraphDraw(const GraphConfig_t* config);
void hkGraphDrawValue(const GraphConfig_t* config, const f32 data, const u8 precision);
void hkGraphDrawValueFollow(const GraphConfig_t* config, const f32 data, const u8 precision);