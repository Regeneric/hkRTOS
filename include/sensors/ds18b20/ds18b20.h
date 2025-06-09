#pragma once
#include <defines.h>

#include <comm/onewire/onewire.h>

typedef enum {    
    DS18B20_CONVERT_T         = 0x44,
    DS18B20_WRITE_SCRATCHPAD  = 0x4E,
    DS18B20_COPY_SCRATCHPAD   = 0x48,
    DS18B20_READ_SCRATCHPAD   = 0xBE,
    DS18B20_RECALL_EE         = 0xB8,
    DS18B20_READ_POWER_SUPPLY = 0xB4
} DS18B20_Commands;

typedef struct DS18B20_Config_t {
    u64    address;
    u8*    data;
    size_t length;
    void*  queue;
    f32    temperature;
} DS18B20_Config_t;

u8 DS18B20_Read(OneWire_Config_t* ow, DS18B20_Config_t* config);