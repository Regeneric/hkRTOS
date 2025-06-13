#pragma once
#include <defines.h>

static i32 map(i32 x, i32 inMin, i32 inMax, i32 outMin, i32 outMax) {
    return (x-inMin) * (outMax-outMin)/(inMax-inMin) + outMin;
}