#pragma once
#include <defines.h>

typedef enum {
    BME_INIT,
    BME_READ_SUCCESS,
    BME_READ_IN_PROGRESS,
    BME_READ_BAD_CHECKSUM
} DHT_Status_t;

typedef struct BME_Config_t {
    u8     gpio;
    u8*    data;
    size_t length;
    void*  queue;
    vu8    status;
} BME_Config_t;

void BME_Init(BME_Config_t* config);
b8   BME_Read(BME_Config_t* config);