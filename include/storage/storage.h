#pragma once
#include <defines.h>

typedef struct DataPacket_t {
    u8* data;
    u32 size;
    u32 address;
} DataPacket_t;