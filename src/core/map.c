#include <core/map.h>

u32 map(u32 x, u32 inMin, u32 inMax, u32 outMin, u32 outMax) {
    return (x-inMin) * (outMax-outMin)/(inMax-inMin) + outMin;
}