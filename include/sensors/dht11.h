#pragma once
#include <defines.h>

#define hkDHT_QUEUE_LEN 5

typedef struct DHT_Config_t {
    u8    gpio;
    u8*   data;
    void* queue;
} DHT_Config_t;

void DHT11_Init(const DHT_Config_t* config);
b8   DHT11_Read(DHT_Config_t* config);