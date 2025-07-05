#pragma once
#include <defines.h>

static u8 intToBCD(u8 num) {return ((num/10) << 4) | (num % 10);}
static u8 BCDToInt(u8 num) {return ((num >> 4) * 10) + (num & 0x0F);}