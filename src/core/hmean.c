#include <core/hmean.h>

// Harmonic mean
f32 hmean(const u8* buffer, size_t len) {
    f32 sumInv = 0.0f;
    size_t count = 0;

    for(size_t i = 0; i != len; ++i) {
        if(buffer[i] != 0) {
            sumInv += 1.0f / (f32)buffer[i];
            count++;
        }
    }

    if(count == 0 || sumInv <= 0.0f) return 0.0f;
    return (f32)count / sumInv;
}