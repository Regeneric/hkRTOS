#pragma once
#include <defines.h>

typedef struct KY40_Config_t {
    u8 clk;
    u8 dt;
    u8 btn;
} KY40_Config_t;

void KY40_InitAll(const KY40_Config_t* config);
u8   KY40_Position(u8 index, i8 value);