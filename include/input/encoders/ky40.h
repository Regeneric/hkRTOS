#pragma once
#include <defines.h>

typedef struct KY40_Config_t {
    u8  clk;
    u8  dt;
    u8  button;
    vu8 position;
} KY40_Config_t;

void KY40_Init(KY40_Config_t* config);