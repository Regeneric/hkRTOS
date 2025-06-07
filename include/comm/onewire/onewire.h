#pragma once
#include <defines.h>

#include <hardware/pio.h>

typedef enum {
    ONEW_INIT,
    ONEW_INIT_SUCCESS,
    ONEW_READ_SUCCESS,
    ONEW_READ_IN_PROGRESS,
    ONEW_WRITE_SUCCESS,
    ONEW_WRITE_IN_PROGRESS
} OneWire_Status_t;

typedef struct OneWire_Config_t {
    u8     gpio;
    vu8    status;
    PIO    pio;
    u8     sm;
} OneWire_Config_t;

void OneWire_Init(OneWire_Config_t* config);
b8   OneWire_Reset(OneWire_Config_t* config);
b8   OneWire_Read(OneWire_Config_t* config, u8* buffer, size_t length);
b8   OneWire_WriteByte(OneWire_Config_t* config, u8 data);
b8   OneWire_Write(OneWire_Config_t* config, u8* buffer, size_t length);
