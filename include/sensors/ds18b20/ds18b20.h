#pragma once
#include <defines.h>

#include <comm/onewire/onewire.h>

typedef enum {
    DS18B20_SKIP_ROM  = 0xCC,
    DS18B20_MATCH_ROM = 0x55,
    DS18B20_READ_SCRATCHPAD = 0xBE
} DS18B20_Commands;

typedef struct DS18B20_Config_t {
    u64    address;
    u8*    data;
    size_t length;
    void*  queue;
} DS18B20_Config_t;

b8 DS18B20_Read(OneWire_Config_t* ow, DS18B20_Config_t* config);